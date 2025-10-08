#!/usr/bin/env python3
import socket, struct

PI_IP    = "10.167.152.2"     # 라즈베리파이 IP
TC_IP    = "10.167.152.20"    # TC375 IP
SD_PORT  = 30490              # SOME/IP-SD (유니캐스트로 PSDD 응답 받음)

def build_findservice():
    service_id = 0xFFFF
    method_id  = 0x8100
    req_id     = 0x00000001
    proto_ver  = 0x01
    iface_ver  = 0x01
    msg_type   = 0x00  # REQUEST
    ret_code   = 0x00

    sd_payload = b"\x00" * 16  # TC375 콜백 트리거용 최소 길이
    length = 8 + len(sd_payload)

    hdr  = struct.pack(">HHI", service_id, method_id, length)
    rest = struct.pack(">IBBBB", req_id, proto_ver, iface_ver, msg_type, ret_code)
    return hdr + rest + sd_payload

def parse_psdd_packet(data: bytes):
    if len(data) < 8 or data[0:4] != b'PSDD':
        return None
    ver   = data[4]
    count = data[5]
    off = 8
    items = []
    for _ in range(count):
        if off + 12 > len(data): return None
        svc   = struct.unpack(">H", data[off+0:off+2])[0]
        inst  = struct.unpack(">H", data[off+2:off+4])[0]
        major = data[off+4]
        minor = struct.unpack(">I", data[off+5:off+9])[0]
        proto = data[off+9]
        port  = struct.unpack(">H", data[off+10:off+12])[0]
        off  += 12
        if off + 1 > len(data): return None
        nlen  = data[off]; off += 1
        if off + nlen > len(data): return None
        name  = data[off:off+nlen].decode(errors="ignore")
        off  += nlen
        items.append({
            "service": svc, "instance": inst, "major": major, "minor": minor,
            "proto": proto, "port": port, "name": name
        })
    return {"ver": ver, "services": items}

def main():
    pkt = build_findservice()
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.bind((PI_IP, 0))  # 임의 소스포트로 바인드
        s.settimeout(2.0)
        s.sendto(pkt, (TC_IP, SD_PORT))
        print(f"[SD] FindService -> {TC_IP}:{SD_PORT} (from {s.getsockname()})")

        try:
            data, addr = s.recvfrom(2048)
            print(f"[SD] got {len(data)} bytes from {addr}")
            parsed = parse_psdd_packet(data)
            if not parsed:
                print("[SD] PSDD parse failed. dump:", data[:64].hex())
                return
            print(f"[SD] version={parsed['ver']}  services={len(parsed['services'])}")
            for i, it in enumerate(parsed["services"], 1):
                print(f"  - [{i}] svc=0x{it['service']:04X}, inst=0x{it['instance']:04X}, "
                      f"major={it['major']}, proto={'UDP' if it['proto']==0 else 'TCP'}, "
                      f"port={it['port']}, name={it['name']}")
        except socket.timeout:
            print("[SD] no PSDD reply (timeout)")

if __name__ == "__main__":
    main()

