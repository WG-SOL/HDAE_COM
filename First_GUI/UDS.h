#ifndef UDS_H
#define UDS_H
#include <stdint.h>
#include <stdbool.h>
#include "Did_Type.h"

// SID 정의
#define SID_READ_DATA_BY_ID  0x22
#define SID_WRITE_DATA_BY_ID 0x2E

// UDS 처리 함수
void uds_process_request(const uint8_t* req, uint16_t len, void* tpcb);




#endif // UDS_H

