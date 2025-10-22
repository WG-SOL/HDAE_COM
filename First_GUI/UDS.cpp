#include "UDS.h"
#include <stdio.h>  // printf
#include <string.h> // memcpy

#define TCP_WRITE_FLAG_COPY 0

void uds_process_request(const uint8_t* req, uint16_t len, void* tpcb) {
    if(len < 3) return;

    uint8_t service_id = req[0];
    uint16_t data_id   = (req[1] << 8) | req[2];

    if(service_id == SID_READ_DATA_BY_ID) {
        // DID 테이블 검색
        const DID_Table_t* didEntry = 0;
        for(int i = 0; i < DID_TABLE_SIZE; i++) {
            if(didTable[i].did == data_id) {
                didEntry = &didTable[i];
                break;
            }
        }
        if(!didEntry) return; // DID 없음

        uint8_t dataBuf[256] = {0};
        uint8_t dataLen = didEntry->readFunc(dataBuf);

        // UDS 응답 생성 (간단 예시)
        uint8_t resp[260];
        resp[0] = 0x62; // Positive Response
        resp[1] = req[1]; // DID High
        resp[2] = req[2]; // DID Low
        memcpy(&resp[3], dataBuf, dataLen);

        //tcp_write(tpcb, resp, 3 + dataLen, TCP_WRITE_FLAG_COPY);
    }
    else if(service_id == SID_WRITE_DATA_BY_ID) {
        // Write 처리 예시
        // 실제 구현 필요
    }
}
