#include <iostream>
#include <thread>
#include <atomic>
#include <csignal>
#include <cstring>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <cctype>
#include <cstdint>

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

using namespace std;

// ====== 환경 상수 ======
static const char* PI_IP        = "192.168.2.10";

// 제어 TC375(모터)
static const char* CTRL_IP      = "192.168.2.30";
static const int   CTRL_PORT    = 30509;

// 바디 TC375(램프)
static const char* BODY_IP      = "192.168.2.20";
static const int   BODY_PORT    = 30509;

// SOME/IP 공통
static const uint16_t CLIENT_ID  = 0x1111;
static const uint8_t  IFACE_VER  = 0x01;
static const uint8_t  PROTO_VER  = 0x01;
static const uint8_t  MSG_TYPE_REQUEST = 0x00;
static const uint8_t  RET_OK = 0x00;

// === ID 규칙 (요청대로) ===
// Service ID는 동일(0x0100) 사용
static const uint16_t SERVICE_ID_COMMON = 0x0100;
// 모터: 기존 유지
static const uint16_t METHOD_ID_MOTOR   = 0x0201;
// 램프: 새로 0x0301
static const uint16_t METHOD_ID_LIGHT   = 0x0301;

// ====== 전역 ======
atomic<bool> g_running{true};

// ====== 안전 큐 ======
class SafeQueue {
public:
    void push(string s) {
        lock_guard<mutex> lk(mu_);
        q_.push_back(move(s));
        cv_.notify_one();
    }
    bool pop(string &out) {
        unique_lock<mutex> lk(mu_);
        cv_.wait(lk, [&]{ return !q_.empty() || !g_running.load(); });
        if (!g_running.load() && q_.empty()) return false;
        out = move(q_.front());
        q_.pop_front();
        return true;
    }
    void stop() {
        lock_guard<mutex> lk(mu_);
        cv_.notify_all();
    }
private:
    mutex mu_;
    condition_variable cv_;
    deque<string> q_;
};
SafeQueue g_txq;

// ====== 문자열 유틸 ======
static inline void trim_inplace(string &s){
    size_t b = s.find_first_not_of(" \t\r\n");
    size_t e = s.find_last_not_of(" \t\r\n");
    if (b == string::npos) { s.clear(); return; }
    s = s.substr(b, e - b + 1);
}
static vector<string> split_sc(const string& line){
    vector<string> t;
    string cur;
    for(char c: line){
        if(c==';'){ trim_inplace(cur); t.push_back(cur); cur.clear(); }
        else cur.push_back(c);
    }
    trim_inplace(cur);
    if(!cur.empty() || (!t.empty() && !line.empty() && line.back()==';')) t.push_back(cur);
    return t;
}
static bool to_int(const string& s, int& out){
    if(s.empty()) return false;
    try { size_t p=0; long v = stol(s,&p,10); if(p!=s.size()) return false; out=(int)v; return true; }
    catch(...){ return false; }
}
static bool parse_dir(const string& s, int& dir){ // 1=전,0=정지,-1=후
    string t=s; for(char& c: t) c=(char)tolower(c);
    if(t=="f"||t=="forward") { dir= 1; return true; }
    if(t=="b"||t=="back"||t=="reverse") { dir=-1; return true; }
    if(to_int(t, dir)) { dir = (dir>0)?1:((dir<0)?-1:0); return true; }
    return false;
}

// 입력(정확히 7필드): Rspd;Lspd;LampL;LampR;Hazard;Rdir;Ldir
// 모터용 출력(4필드): "Rspd;Lspd;Rdir;Ldir"
static bool reduce_to_motor4_exact7(const string& line, string& out4) {
    auto t = split_sc(line);
    if(t.size() != 7) return false;

    int Rspd=0, Lspd=0;
    if(!to_int(t[0], Rspd)) return false;
    if(!to_int(t[1], Lspd)) return false;

    int Rdir=0, Ldir=0;
    if(!parse_dir(t[5], Rdir)) return false;
    if(!parse_dir(t[6], Ldir)) return false;

    out4 = to_string(Rspd) + ";" +
           to_string(Lspd) + ";" +
           to_string(Rdir) + ";" +
           to_string(Ldir);
    return true;
}

// 램프용 출력(3필드): "LampL;LampR;Hazard"
static bool reduce_to_lamp3_exact7(const string& line, string& out3) {
    auto t = split_sc(line);
    if(t.size() != 7) return false;

    int L=0, R=0, H=0;
    if(!to_int(t[2], L)) return false;  // LampL
    if(!to_int(t[3], R)) return false;  // LampR
    if(!to_int(t[4], H)) return false;  // Hazard

    // 0/1로 클램핑
    L = L ? 1 : 0; R = R ? 1 : 0; H = H ? 1 : 0;

    out3 = to_string(L) + ";" + to_string(R) + ";" + to_string(H);
    return true;
}

// ====== SOME/IP 송신기 ======
class SomeipSender {
public:
    SomeipSender() : sock_(-1), session_(1) {}

    bool open_to(const char* dst_ip, int dst_port) {
        sock_ = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (sock_ < 0) { perror("udp socket"); return false; }

        sockaddr_in src{}; src.sin_family = AF_INET;
        src.sin_addr.s_addr = inet_addr(PI_IP); // 필요시 INADDR_ANY
        src.sin_port = htons(0);
        if (bind(sock_, (sockaddr*)&src, sizeof(src)) < 0) {
            perror("udp bind"); ::close(sock_); sock_ = -1; return false;
        }

        memset(&dst_, 0, sizeof(dst_));
        dst_.sin_family = AF_INET;
        dst_.sin_addr.s_addr = inet_addr(dst_ip);
        dst_.sin_port = htons(dst_port);
        return true;
    }

    void closeSock() {
        if (sock_ >= 0) ::close(sock_);
        sock_ = -1;
    }

    bool sendRequest(const vector<uint8_t>& payload,
                     uint16_t service_id,
                     uint16_t method_id) {
        if (sock_ < 0) return false;

        uint32_t length = 8 + payload.size();
        uint32_t req_id = (uint32_t(CLIENT_ID) << 16) | (session_.fetch_add(1) & 0xFFFF);

        vector<uint8_t> pkt;
        pkt.resize(16 + payload.size());

        auto be32 = [](uint8_t* p, uint32_t v){ p[0]=uint8_t(v>>24); p[1]=uint8_t(v>>16); p[2]=uint8_t(v>>8); p[3]=uint8_t(v); };

        // Message ID = (ServiceID<<16) | MethodID
        uint32_t msg_id = (uint32_t(service_id) << 16) | uint32_t(method_id);
        be32(&pkt[0], msg_id);
        be32(&pkt[4], length);
        be32(&pkt[8], req_id);
        pkt[12] = PROTO_VER;
        pkt[13] = IFACE_VER;
        pkt[14] = MSG_TYPE_REQUEST;
        pkt[15] = RET_OK;

        if (!payload.empty())
            memcpy(&pkt[16], payload.data(), payload.size());

        ssize_t sent = ::sendto(sock_, pkt.data(), pkt.size(), 0,
                                (sockaddr*)&dst_, sizeof(dst_));
        if (sent < 0) { perror("sendto"); return false; }
        return true;
    }

private:
    int sock_;
    sockaddr_in dst_{};
    atomic<uint32_t> session_;
};

// ====== RFCOMM 서버 ======
int setup_rfcomm(uint8_t channel) {
    int sock = ::socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (sock < 0) { perror("rfcomm socket"); return -1; }

    sockaddr_rc loc{}; loc.rc_family = AF_BLUETOOTH;
    loc.rc_channel = channel;
    memset(&loc.rc_bdaddr, 0, sizeof(bdaddr_t)); // BDADDR_ANY

    if (::bind(sock, (struct sockaddr*)&loc, sizeof(loc)) < 0) {
        perror("rfcomm bind"); ::close(sock); return -1;
    }
    if (::listen(sock, 1) < 0) {
        perror("rfcomm listen"); ::close(sock); return -1;
    }
    return sock;
}

void bt_accept_loop(int server_sock) {
    while (g_running.load()) {
        sockaddr_rc rem{}; socklen_t opt = sizeof(rem);
        int cs = ::accept(server_sock, (struct sockaddr*)&rem, &opt);
        if (cs < 0) {
            if (!g_running.load()) break;
            continue;
        }
        char addr[18]{0}; ba2str(&rem.rc_bdaddr, addr);
        cout << "[BT] client connected: " << addr << endl;

        thread([cs]{
            char buf[1024];
            while (g_running.load()) {
                ssize_t len = ::read(cs, buf, sizeof(buf)-1);
                if (len <= 0) break;
                buf[len] = 0;

                string cmd(buf);
                auto pos = cmd.find_last_not_of(" \r\n\t");
                if (pos != string::npos) cmd.erase(pos+1);
                else cmd.clear();

                if (!cmd.empty())
                    g_txq.push(cmd);
            }
            ::close(cs);
            cout << "[BT] client disconnected\n";
        }).detach();
    }
}

// ====== 송신 스레드: 큐 → SOME/IP(UDP) ======
void someip_tx_loop() {
    SomeipSender tx_ctrl, tx_body;
    if (!tx_ctrl.open_to(CTRL_IP, CTRL_PORT)) {
        cerr << "[SOME/IP] open ctrl failed\n";
        return;
    }
    if (!tx_body.open_to(BODY_IP, BODY_PORT)) {
        cerr << "[SOME/IP] open body failed\n";
        return;
    }
    cout << "[SOME/IP] UDP ready -> CTRL " << CTRL_IP << ":" << CTRL_PORT
         << " | BODY " << BODY_IP << ":" << BODY_PORT << "\n";

    string msg;
    while (g_running.load() && g_txq.pop(msg)) {
        string motor4, lamp3;
        bool ok_motor = reduce_to_motor4_exact7(msg, motor4);
        bool ok_lamp  = reduce_to_lamp3_exact7(msg, lamp3);

        if (!ok_motor && !ok_lamp) {
            cerr << "[SOME/IP] bad frame: " << msg << "\n";
            continue;
        }

        // 모터 → 0x0100 / 0x0201 (기존 유지)
        if (ok_motor) {
            vector<uint8_t> payload(motor4.begin(), motor4.end());
            if (!tx_ctrl.sendRequest(payload, SERVICE_ID_COMMON, METHOD_ID_MOTOR)) {
                cerr << "[SOME/IP] send motor failed\n";
            }
        }

        // 램프 → 0x0100 / 0x0301 (신규)
        if (ok_lamp) {
            vector<uint8_t> payload(lamp3.begin(), lamp3.end());
            if (!tx_body.sendRequest(payload, SERVICE_ID_COMMON, METHOD_ID_LIGHT)) {
                cerr << "[SOME/IP] send light failed\n";
            }
        }
        // 필요시 디바운스
        // this_thread::sleep_for(chrono::milliseconds(1));
    }

    tx_ctrl.closeSock();
    tx_body.closeSock();
}

// ====== 종료 처리 ======
void on_sigint(int) {
    g_running.store(false);
    g_txq.stop();
    cout << "\n[SYS] shutting down...\n";
    ::exit(0);
}

int main() {
    signal(SIGINT, on_sigint);

    uint8_t channel = 1;
    int s = setup_rfcomm(channel);
    if (s < 0) return 1;
    cout << "[BT] RFCOMM listening on channel " << (int)channel << "\n";

    thread th_accept(bt_accept_loop, s);
    thread th_tx(someip_tx_loop);

    th_accept.join();
    th_tx.join();

    ::close(s);
    return 0;
}

