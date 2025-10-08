#include <vsomeip/vsomeip.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <cstring>

#define SERVICE_ID   0x0100
#define INSTANCE_ID  0x0001
#define METHOD_ID    0x0101

static std::shared_ptr<vsomeip::application> app;
static std::atomic<bool> service_available{false};
static std::atomic<bool> request_sent{false};

/* 0x01=ON, 0x00=OFF */
static vsomeip::byte_t parse_onoff_from_argv(int argc, char **argv) {
    if (argc >= 2) {
        if (!std::strcmp(argv[1], "on"))  return 0x01;
        if (!std::strcmp(argv[1], "off")) return 0x00;
    }
    return 0x01; // default: ON
}

static void send_request_once(vsomeip::byte_t onoff) {
    if (request_sent.exchange(true)) {
        return; // 이미 한 번 보냈음
    }
    auto rq = vsomeip::runtime::get()->create_request();
    rq->set_service(SERVICE_ID);
    rq->set_instance(INSTANCE_ID);
    rq->set_method(METHOD_ID);

    std::vector<vsomeip::byte_t> pl = { onoff }; // payload 1바이트
    rq->set_payload(vsomeip::runtime::get()->create_payload(pl));

    std::cout << "[Pi] send LED " << (onoff ? "ON" : "OFF") << std::endl;
    app->send(rq);
}

int main(int argc, char **argv) {
    vsomeip::byte_t onoff = parse_onoff_from_argv(argc, argv);

    app = vsomeip::runtime::get()->create_application("client-sample");
    if (!app->init()) {
        std::cerr << "init failed\n";
        return 1;
    }

    // 1) 서비스 가용성 핸들러 (SD가 켜져 있을 때만 의미 있음)
    app->register_availability_handler(
        SERVICE_ID, INSTANCE_ID,
        [onoff](vsomeip::service_t, vsomeip::instance_t, bool available){
            service_available.store(available);
            std::cout << "[Pi] availability: " << (available ? "AVAILABLE" : "NOT AVAILABLE") << std::endl;
            if (available) {
                // SD로 라우팅 정보가 잡히면 약간의 여유 후 전송
                std::thread([onoff]{
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    send_request_once(onoff);
                }).detach();
            }
        });

    // 2) 응답 핸들러
    app->register_message_handler(SERVICE_ID, INSTANCE_ID, METHOD_ID,
        [](const std::shared_ptr<vsomeip::message> &resp){
            auto pl = resp->get_payload();
            std::cout << "[Pi] got response, len=" << (pl ? pl->get_length() : 0) << std::endl;
            app->stop();
        });

    // 3) 앱 상태 핸들러
    app->register_state_handler([onoff](vsomeip::state_type_e s){
        if (s == vsomeip::state_type_e::ST_REGISTERED) {
            std::cout << "[Pi] app registered\n";

            // SD 경로: FindService 요청 → Offer 대기
            app->request_service(SERVICE_ID, INSTANCE_ID);

            // ★ SD가 꺼져있거나(정적 라우팅) AVAILABLE이 오지 않는 환경 대비: 300ms 후 강제 1회 송신
            std::thread([onoff]{
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                // SD 없는 모드에서도 바로 전송 (라우팅은 JSON 정적 설정으로 해결)
                send_request_once(onoff);
            }).detach();
        }
    });

    app->start();
    return 0;
}

