#!/usr/bin/env python3
import os, time, zipfile, shutil, tempfile, subprocess
from pathlib import Path
from flask import Flask, request, jsonify, abort, send_from_directory

ROOT      = Path("/opt/traffic-app")
RELEASES  = ROOT / "releases"
CURRENT   = ROOT / "current"           # symlink -> releases/<ver>
STATE     = ROOT / "state"
APP_FILE  = CURRENT / "app.py"         # 단일 파일 교체 타겟

SEARCH_CANDIDATES = [
    ".",                      # ZIP 최상단
    "web/web_base",           # 원규님 ZIP 구조 고려
    "web", "src", "app",      # 혹시 모를 경로
]

app = Flask(__name__)

def fsync_dir(d: Path):
    fd = os.open(str(d), os.O_DIRECTORY); os.fsync(fd); os.close(fd)

def atomic_replace_file(target: Path, data: bytes, mode=0o755):
    fd, tmp = tempfile.mkstemp(prefix=".swap_", dir=str(target.parent))
    try:
        with os.fdopen(fd, "wb") as f:
            f.write(data); f.flush(); os.fsync(f.fileno())
        os.chmod(tmp, mode)
        os.replace(tmp, target)            # 같은 파티션 내 원자 교체
        fsync_dir(target.parent)
    finally:
        if os.path.exists(tmp): os.unlink(tmp)

def atomic_switch_current(to_dir: Path):
    tmp = ROOT / "current.new"
    if tmp.exists(): tmp.unlink()
    os.symlink(to_dir, tmp)
    tmp.replace(CURRENT)                    # 링크 원자 교체
    fsync_dir(ROOT)

def restart_app_service():
    subprocess.run(["/bin/systemctl", "restart", "traffic-app.service"], check=False)

def find_app_py(base_dir: Path) -> Path:
    # 지정된 후보 경로들에서 app.py 탐색
    for rel in SEARCH_CANDIDATES:
        p = base_dir / rel / "app.py"
        if p.exists():
            return p
    # 마지막으로 전체 탐색
    for p in base_dir.rglob("app.py"):
        return p
    return None

@app.route("/upload", methods=["POST"])
def upload():
    """
    폼:
      - file: app.py 또는 ZIP
      - version: (선택) ZIP일 때 releases/<version> 폴더명
      - mode: (선택) 'single' 또는 'zip' (미지정 시 확장자 기반 자동)
    """
    f = request.files.get("file")
    if not f:
        abort(400, "no file")
    filename = f.filename or ""
    mode = (request.form.get("mode") or "").strip().lower()
    version = (request.form.get("version") or "").strip()

    data = f.read()
    if not mode:
        mode = "zip" if filename.lower().endswith(".zip") else "single"

    if mode == "single":
        # 단일 파일 스왑
        print(f"[HTTP-OTA] single: {filename}, {len(data)} bytes")
        atomic_replace_file(APP_FILE, data, 0o755)
        restart_app_service()
        return jsonify({"ok": True, "applied": "single", "target": str(APP_FILE)})

    elif mode == "zip":
        print(f"[HTTP-OTA] zip: {filename}, {len(data)} bytes")
        # 릴리즈 폴더명 결정
        if not version:
            base = filename.rsplit(".",1)[0] or f"r{int(time.time())}"
            version = base
        dest = RELEASES / version
        if dest.exists():
            shutil.rmtree(dest)
        dest.mkdir(parents=True, exist_ok=True)

        # ZIP 저장 및 전개
        zpath = dest / "bundle.zip"
        zpath.write_bytes(data)
        with zipfile.ZipFile(zpath, "r") as z:
            z.extractall(dest)

        # app.py 자동 탐색
        app_py = find_app_py(dest)
        if not app_py:
            shutil.rmtree(dest, ignore_errors=True)
            abort(400, "zip must contain app.py (auto-search failed)")

        # 최상단에 없을 경우, 실행 경로로 복사해 맞춰줌
        if app_py.parent != dest:
            shutil.copy2(app_py, dest / "app.py")

        os.chmod(dest / "app.py", 0o755)
        print(f"[HTTP-OTA] switch current -> {dest}")
        atomic_switch_current(dest)
        restart_app_service()
        return jsonify({"ok": True, "applied": "zip", "current": str(dest)})

    else:
        abort(400, "invalid mode")

@app.route("/files/<path:name>", methods=["GET"])
def files(name):
    # (옵션) 확인용 서빙
    store = ROOT / "releases"
    return send_from_directory(store, name, as_attachment=True)

if __name__ == "__main__":
    RELEASES.mkdir(parents=True, exist_ok=True)
    STATE.mkdir(parents=True, exist_ok=True)
    app.run(host="0.0.0.0", port=8080)
