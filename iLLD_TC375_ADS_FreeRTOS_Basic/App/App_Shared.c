#include "App_Shared.h"
#include <stdio.h>

#define BLE_CMD_FIELD_COUNT    (4)
#define BLE_DUTY_DEADBAND      (5)
#define DUTY_MIN               (0)
#define DUTY_MAX               (100)

static DriveCommand g_latestCommand;
static TofSample g_latestTof;
static bool g_aebActive;

static inline int clampi(int value, int lo, int hi)
{
    if (value < lo)
    {
        return lo;
    }
    if (value > hi)
    {
        return hi;
    }
    return value;
}

void AppShared_Init(void)
{
    taskENTER_CRITICAL();
    g_latestCommand.valid = false;
    g_latestCommand.timestamp = 0;
    g_latestCommand.left_duty = 0;
    g_latestCommand.right_duty = 0;
    g_latestCommand.left_dir = 1;
    g_latestCommand.right_dir = 1;

    g_latestTof.valid = false;
    g_latestTof.distance_cm = 0.0f;
    g_latestTof.timestamp = 0;

    g_aebActive = false;
    taskEXIT_CRITICAL();
}

bool AppShared_ParseBleCommand(const char *line, DriveCommand *out_cmd)
{
    if ((line == NULL) || (out_cmd == NULL))
    {
        return false;
    }

    int left = 0;
    int right = 0;
    //int swL = 0, swR = 0, swH = 0, swP = 0, swLK = 0;
    int left_dir = 1;
    int right_dir = 1;

    int parsed = sscanf(line, "%d;%d;%d;%d",
                        &left, &right, &left_dir, &right_dir);

    if (parsed != BLE_CMD_FIELD_COUNT)
    {
        return false;
    }

    if (left < DUTY_MIN || left > 1000 || right < DUTY_MIN || right > 1000)
    {
        return false;
    }

    DriveCommand cmd = {0};
    cmd.left_duty = (left < BLE_DUTY_DEADBAND) ? 0 : clampi(left, DUTY_MIN, DUTY_MAX);
    cmd.right_duty = (right < BLE_DUTY_DEADBAND) ? 0 : clampi(right, DUTY_MIN, DUTY_MAX);
    cmd.left_dir = (left_dir != 0) ? 1 : 0;
    cmd.right_dir = (right_dir != 0) ? 1 : 0;
    cmd.timestamp = xTaskGetTickCount();
    cmd.valid = true;

    *out_cmd = cmd;
    return true;
}

void AppShared_SetCommand(const DriveCommand *cmd)
{
    if (cmd == NULL)
    {
        return;
    }

    taskENTER_CRITICAL();
    g_latestCommand = *cmd;
    taskEXIT_CRITICAL();
}

bool AppShared_GetCommand(DriveCommand *out_cmd)
{
    if (out_cmd == NULL)
    {
        return false;
    }

    taskENTER_CRITICAL();
    *out_cmd = g_latestCommand;
    taskEXIT_CRITICAL();

    return g_latestCommand.valid;
}

bool AppShared_GetCommandIfFresh(DriveCommand *out_cmd, TickType_t now, TickType_t stale_timeout_ticks)
{
    if (!AppShared_GetCommand(out_cmd))
    {
        return false;
    }

    if ((now - out_cmd->timestamp) > stale_timeout_ticks)
    {
        return false;
    }

    return true;
}

void AppShared_UpdateTof(const TofSample *sample)
{
    if (sample == NULL)
    {
        return;
    }

    taskENTER_CRITICAL();
    g_latestTof = *sample;
    taskEXIT_CRITICAL();
}

bool AppShared_GetTof(TofSample *out_sample)
{
    if (out_sample == NULL)
    {
        return false;
    }

    taskENTER_CRITICAL();
    *out_sample = g_latestTof;
    taskEXIT_CRITICAL();

    return g_latestTof.valid;
}

void AppShared_UpdateTofFromIsr(const TofSample *sample)
{
    if (sample == NULL)
    {
        return;
    }

    UBaseType_t mask = taskENTER_CRITICAL_FROM_ISR();
    g_latestTof = *sample;
    taskEXIT_CRITICAL_FROM_ISR(mask);
}

void AppShared_SetAebActive(bool active)
{
    taskENTER_CRITICAL();
    g_aebActive = active;
    taskEXIT_CRITICAL();
}

bool AppShared_IsAebActive(void)
{
    bool result;
    taskENTER_CRITICAL();
    result = g_aebActive;
    taskEXIT_CRITICAL();
    return result;
}

void AppShared_SetAebActiveFromIsr(bool active)
{
    UBaseType_t mask = taskENTER_CRITICAL_FROM_ISR();
    g_aebActive = active;
    taskEXIT_CRITICAL_FROM_ISR(mask);
}
