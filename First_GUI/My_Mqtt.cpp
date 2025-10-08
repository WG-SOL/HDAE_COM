#define _CRT_SECURE_NO_WARNINGS

#include <My_Mqtt.h>
#include <filesystem>
namespace fs = std::filesystem;
MqttHandler::MqttHandler(const char* id, QObject *parent)
    : QObject(parent), mosqpp::mosquittopp(id)
{
    mosqpp::lib_init(); // 라이브러리 초기화

    // loop_start()는 내부적으로 스레드를 생성하여 백그라운드에서
    // 네트워크 통신을 처리합니다. GUI를 멈추지 않게 하는 핵심입니다.
    loop_start();
}

MqttHandler::~MqttHandler()
{
    loop_stop(true); // 스레드 정리
    mosqpp::lib_cleanup();
}

void MqttHandler::connectToBroker(const char* host, int port)
{
    connect_async(host, port, 60); // 비동기 방식으로 연결
}

void MqttHandler::publishMessage(const std::string& topic, const std::string& message)
{
    publish(NULL, topic.c_str(), message.length(), message.c_str());
}

// 파일을 전송하는 함수
void MqttHandler::sendFile(const std::string& topic, const std::string& file_path)
{
    // 1. 파일 존재 여부 및 크기 확인
    if (!fs::exists(file_path)) {
        std::cerr << "Error: File not found at " << file_path << std::endl;
        return;
    }
    long long file_size = fs::file_size(file_path);
    std::string filename = fs::path(file_path).filename().string();
    const size_t chunk_size = 4096; // 4KB

    // 2. 파일 전송 시작 메시지 발행
    std::string start_msg = "START," + filename + "," + std::to_string(file_size);
    publish(NULL, topic.c_str(), start_msg.length(), start_msg.c_str());
    std::cout << "Sent START signal: " << start_msg << std::endl;

    // 3. 파일을 열고 조각내어 발행
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file." << std::endl;
        return;
    }

    std::vector<char> buffer(chunk_size);
    while (!file.eof()) {
        file.read(buffer.data(), chunk_size);
        size_t bytes_read = file.gcount();
        if (bytes_read > 0) {
            // 자신의 publish 함수를 호출합니다. loop()는 필요 없습니다.
            publish(NULL, topic.c_str(), bytes_read, buffer.data());
        }
    }
    file.close();

    // 4. 파일 전송 종료 메시지 발행
    // 백그라운드 루프가 이전 조각들을 보낼 시간을 벌어주기 위해 약간의 지연을 줍니다.
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::string end_msg = "END";
    publish(NULL, topic.c_str(), end_msg.length(), end_msg.c_str());
    std::cout << "Sent END signal." << std::endl;
}


// --- 콜백 함수 구현 ---

void MqttHandler::on_connect(int rc)
{
    if (rc == 0) {
        std::cout << "MQTT Handler: Connected to broker." << std::endl;
        m_isConnected = true;
        emit connected(); // 연결 성공 시그널 발생
    } else {
        std::cout << "MQTT Handler: Connection failed." << std::endl;
        m_isConnected = false;
    }
}

void MqttHandler::on_disconnect(int rc)
{
    std::cout << "MQTT Handler: Disconnected." << std::endl;
    m_isConnected = false;
    emit disconnected(); // 연결 끊김 시그널 발생
}

bool MqttHandler::isConnected() const
{
    return m_isConnected;
}
