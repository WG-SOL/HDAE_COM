#include "lwip/opt.h"
#include "lwip/tcp.h"
#include "string.h"
#include "DoIP.h"
#include "ToF.h" // ToF.h 헤더 파일 포함

/* --- UDS 서비스 및 DID 정의 --- */
#define SID_READ_DATA_BY_ID 0x22
#define DID_TOF             0x0100

#if LWIP_TCP

/* --- 함수 프로토타입 선언 --- */
static err_t doip_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t doip_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static void doip_error(void *arg, err_t err);

#define UDS_PORT 13400

void DoIP_Init(void)
{
    struct tcp_pcb *pcb = tcp_new();
    if (pcb != NULL) {
        err_t err = tcp_bind(pcb, IP_ADDR_ANY, UDS_PORT);
        if (err == ERR_OK) {
            pcb = tcp_listen(pcb);
            tcp_accept(pcb, doip_accept);
        } else {
            tcp_abort(pcb);
        }
    }
}

static err_t doip_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    LWIP_UNUSED_ARG(arg);
    LWIP_UNUSED_ARG(err);
    tcp_recv(newpcb, doip_recv);
    tcp_err(newpcb, doip_error);
    return ERR_OK;
}

static err_t doip_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (p == NULL) {
        tcp_close(tpcb);
        return ERR_OK;
    }
    if (err != ERR_OK) {
        pbuf_free(p);
        return err;
    }

    uint16_t payload_type = (uint16_t)(((uint8_t*)p->payload)[2] << 8) | ((uint8_t*)p->payload)[3];

    if (payload_type == 0x0005) // Routing Activation Request
    {
        uint8_t resp[] = {
            0x03, 0xFC, 0x00, 0x06, 0x00, 0x00, 0x00, 0x09,
            ((uint8_t*)p->payload)[8], ((uint8_t*)p->payload)[9],
            0x02, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00
        };
        tcp_write(tpcb, resp, sizeof(resp), TCP_WRITE_FLAG_COPY);
        tcp_output(tpcb);
    }
    else if (payload_type == 0x8001) // Diagnostic Message
    {
        // --- 💡 여기가 수정된 부분입니다: 올바른 파싱 위치 ---
        // DoIP 헤더(8) + SA(2) + TA(2) = 12바이트 뒤부터 UDS 메시지 시작
        uint8_t service_id = ((uint8_t*)p->payload)[12];
        uint16_t data_id = (uint16_t)(((uint8_t*)p->payload)[13] << 8) | ((uint8_t*)p->payload)[14];
        // ----------------------------------------------------

        if (service_id == SID_READ_DATA_BY_ID && data_id == DID_TOF)
        {
            uint16_t distance = Tof_GetValue();

            uint8_t resp[] = {
                0x03, 0xFC, 0x80, 0x01, 0x00, 0x00, 0x00, 0x09, // DoIP Header
                ((uint8_t*)p->payload)[10], ((uint8_t*)p->payload)[11], // Target Address (요청의 SA를 복사)
                ((uint8_t*)p->payload)[8], ((uint8_t*)p->payload)[9],   // Source Address (요청의 TA를 복사)
                0x62,                         // UDS Positive Response SID
                (uint8_t)(data_id >> 8),      // DID High
                (uint8_t)(data_id),           // DID Low
                (uint8_t)(distance >> 8),     // 거리 값 High
                (uint8_t)(distance)           // 거리 값 Low
            };
            tcp_write(tpcb, resp, sizeof(resp), TCP_WRITE_FLAG_COPY);
            tcp_output(tpcb);
        }
    }

    tcp_recved(tpcb, p->tot_len);
    pbuf_free(p);

    return ERR_OK;
}

static void doip_error(void *arg, err_t err)
{
    LWIP_UNUSED_ARG(arg);
    LWIP_UNUSED_ARG(err);
}

#endif /* LWIP_TCP */
