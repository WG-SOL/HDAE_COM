#include "App_AEB.h"

#include "App_Shared.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "can.h"
#include "my_stdio.h"

#include <stdint.h>
#include <string.h>

#define AEB_STOP_THRESHOLD_CM    (30.0f)
#define AEB_CLEAR_THRESHOLD_CM   (40.0f)
#define AEB_TASK_STACK_WORDS     (configMINIMAL_STACK_SIZE + 128U)
#define AEB_TASK_PRIORITY        (7U)
#define AEB_QUEUE_LENGTH         (1U)

typedef struct
{
    unsigned int id;
    uint8_t data[8];
    int len;
    TickType_t timestamp;
} TofFrameMessage;

static QueueHandle_t g_tofQueue = NULL;
static TaskHandle_t g_aebTaskHandle = NULL;
static bool g_aebActiveState = false;

static void AppAEB_ProcessTofFrame(const TofFrameMessage *frame);
static void task_aeb_handler(void *arg);

//AEB state 초기화, CAN 초기화, 필터 설정
void AppAEB_Init(void)
{
    g_aebActiveState = false;
    AppShared_SetAebActive(false);

    /* NOTE: If your hardware uses the mikroBUS CAN transceiver,
     * switch CAN_NODE0 -> CAN_NODE2 below. */
    Can_Init(BD_500K, CAN_NODE0);
    Can_SetFilterRange(0x000U, 0x7FFU);
    //my_printf("AEB: CAN initialized (baud=500k, node=%d)\n", (int)CAN_NODE0);

    if (g_tofQueue == NULL)
    {
        g_tofQueue = xQueueCreate(AEB_QUEUE_LENGTH, sizeof(TofFrameMessage));
        configASSERT(g_tofQueue != NULL);
    }

    if (g_aebTaskHandle == NULL)
    {
        BaseType_t ok = xTaskCreate(task_aeb_handler,
                                    "aeb",
                                    AEB_TASK_STACK_WORDS,
                                    NULL,
                                    AEB_TASK_PRIORITY,
                                    &g_aebTaskHandle);
        configASSERT(ok == pdPASS);
    }
}

void Can_RxUserCallback(unsigned int id, const char *data, int len)
{
    //my_printf("CAN RX id=0x%X len=%d\n", id, len); 
    if ((data == NULL) || (len <= 0) || (g_tofQueue == NULL))
    {
        return;
    }

    TofFrameMessage frame = {0};
    frame.id = id;
    frame.len = (len > (int)sizeof(frame.data)) ? (int)sizeof(frame.data) : len;
    memcpy(frame.data, data, (size_t)frame.len);
    frame.timestamp = xTaskGetTickCountFromISR();

    BaseType_t higherPriorityTaskWoken = pdFALSE;
    xQueueOverwriteFromISR(g_tofQueue, &frame, &higherPriorityTaskWoken);
    if (higherPriorityTaskWoken != pdFALSE)
    {
        portYIELD_FROM_ISR(higherPriorityTaskWoken);
    }
}

static void task_aeb_handler(void *arg)
{
    (void)arg;

    TofFrameMessage frame;

    for (;;)
    {
        if (xQueueReceive(g_tofQueue, &frame, portMAX_DELAY) == pdPASS)
        {
            AppAEB_ProcessTofFrame(&frame);
        }
    }
}

static void AppAEB_ProcessTofFrame(const TofFrameMessage *frame)
{
    if ((frame == NULL) || (frame->len <= 0))
    {
        return;
    }

    unsigned int id = frame->id;
    const uint8_t *data = frame->data;
    int len = frame->len;

    //추후 LKAS 프레임 처리 추가 가능
    if (id == 0x123U)
    {
        return; /* legacy LKAS frame, ignore for AEB */
    }

    if (len < 6)
    {
        return;
    }

    //거리: data[0], data[1], data[2], 신호세기: data[4], data[5]
    uint32_t raw_distance = ((uint32_t)data[2] << 16)
                          | ((uint32_t)data[1] << 8)
                          | (uint32_t)data[0];
    uint16_t strength = ((uint16_t)data[5] << 8) | (uint16_t)data[4];

    if ((strength == 0U) || (raw_distance == 0U))
    {
        return;
    }

    float distance_cm = ((float)raw_distance) / 10.0f;
    // my_printf("AEB ToF distance: %.1f cm (strength=%u)\n",
    //           (double)distance_cm,
    //           (unsigned int)strength);

    bool newState = g_aebActiveState;
    if (!g_aebActiveState && (distance_cm <= AEB_STOP_THRESHOLD_CM))
    {
        newState = true;
    }
    else if (g_aebActiveState && (distance_cm >= AEB_CLEAR_THRESHOLD_CM))
    {
        newState = false;
    }

    if (newState != g_aebActiveState)
    {
        g_aebActiveState = newState;
        AppShared_SetAebActive(newState);
    }

    TofSample sample;
    sample.distance_cm = distance_cm;
    sample.timestamp = frame->timestamp;
    sample.valid = true;
    AppShared_UpdateTof(&sample);
}
