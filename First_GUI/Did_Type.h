#ifndef DID_TYPE_H
#define DID_TYPE_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

typedef struct {
    uint16_t did;              // Data Identifier
    uint8_t dataLength;        // 데이터 길이
    const char* description;   // DID 의미
    uint8_t (*readFunc)(uint8_t* dataOut);
    uint8_t (*writeFunc)(const uint8_t* dataIn);
} DID_Table_t;

extern const DID_Table_t didTable[];
extern const int DID_TABLE_SIZE;

#endif // DID_TYPE_H
