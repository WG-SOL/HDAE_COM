#!/usr/bin/env python3
from flask import Flask, request, render_template_string, jsonify
import requests, os

app = Flask(__name__)

HTML = """
<!doctype html>
<html lang="ko">
<meta charset="utf-8" />
<title>Raspberry Pi OTA Uploader (Local)</title>
<body style="font-family:system-ui;max-width:780px;margin:40px auto">
  <h1>Raspberry Pi OTA</h1>
  <form action="/send" method="post" enctype="multipart/form-data" style="display:grid;gap:12px">
    <label>라즈베리파이 IP:
      <input name="pi_ip" placeholder="예: 192.168.137.187" required style="width:260px">
    </label>
    <label>파일 선택(app.py 또는 zip):
      <input type="file" name="file" required>
    </label>
    <div>
      모드:
      <label><input type="radio" name="mode" value="single" checked> 단일 파일(app.py 교체)</label>
      <label><input type="radio" name="mode" value="zip"> ZIP 릴리즈</label>
    </div>
    <label>버전(선택, ZIP일 때 releases/<em>버전</em> 폴더명):
      <input name="version" placeholder="예: 1.2.3" style="width:160px">
    </label>
    <button type="submit">업로드 & 적용</button>
  </form>

  <p style="margin-top:18px;color:#666">
    * 파이의 HTTP OTA 서버는 <code>http://&lt;파이IP&gt;:8080/upload</code> 로 동작합니다.<br>
    * ZIP 내부에 <code>app.py</code>가 루트 또는 <code>web/web_base/</code>에 있으면 자동으로 찾아 적용되게 파이 쪽 서버가 구성되어 있습니다.
  </p>
</body>
</html>
"""

@app.route("/", methods=["GET"])
def index():
    return render_template_string(HTML)

@app.route("/send", methods=["POST"])
def send():
    pi_ip = request.form.get("pi_ip", "").strip()
    mode  = request.form.get("mode", "single").strip()
    version = request.form.get("version", "").strip()
    f = request.files.get("file")
    if not pi_ip or not f:
        return "파이 IP나 파일이 없습니다.", 400

    url = f"http://{pi_ip}:8080/upload"
    filename = f.filename or "upload.bin"
    files = {"file": (filename, f.stream.read())}
    data = {"mode": mode}
    if version:
        data["version"] = version

    try:
        r = requests.post(url, files=files, data=data, timeout=180)
        return f"<pre>HTTP {r.status_code}\n{r.text}</pre>"
    except Exception as e:
        return f"<pre>전송 실패: {e}</pre>", 500

if __name__ == "__main__":
    # 127.0.0.1:5000 에서 서비스
    app.run(host="127.0.0.1", port=5000, debug=False)
