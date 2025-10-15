#include "App_Drive.h"

#include "App_Shared.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Motor.h"

#define DRIVE_TASK_PERIOD_MS        (10U)
#define DRIVE_CMD_STALE_MS          (300U)
#define DUTY_MIN                    (0)
#define DUTY_MAX                    (100)
#define DUTY_SLEW_PER_TICK          (5)
#define DRIVE_TASK_STACK_WORDS      (configMINIMAL_STACK_SIZE + 256U)
#define DRIVE_TASK_PRIORITY         (6U)

static TaskHandle_t g_driveTaskHandle = NULL;

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

static void task_motor_control(void *arg)
{
    (void)arg;

    TickType_t lastWake = xTaskGetTickCount();
    const TickType_t periodTicks = pdMS_TO_TICKS(DRIVE_TASK_PERIOD_MS);
    const TickType_t staleTicks = pdMS_TO_TICKS(DRIVE_CMD_STALE_MS);

    int currentLeft = 0;
    int currentRight = 0;
    int currentLeftDir = 1;
    int currentRightDir = 1;

    for (;;)
    {
        vTaskDelayUntil(&lastWake, periodTicks);

        TickType_t now = xTaskGetTickCount();
        DriveCommand cmd;
        bool haveFresh = AppShared_GetCommandIfFresh(&cmd, now, staleTicks);
        bool aebActive = AppShared_IsAebActive();

        int targetLeft = 0;
        int targetRight = 0;
        int targetLeftDir = 1;
        int targetRightDir = 1;

        //cmd가 유효하면
        if (haveFresh)
        {
            targetLeft = clampi(cmd.left_duty, DUTY_MIN, DUTY_MAX);
            targetRight = clampi(cmd.right_duty, DUTY_MIN, DUTY_MAX);
            targetLeftDir = cmd.left_dir ? 1 : 0;
            targetRightDir = cmd.right_dir ? 1 : 0;
        }
        //AEB가 활성화되어있고 전진상태라면 정지
        if (aebActive && (targetLeftDir == 1) && (targetRightDir == 1))
        {
            targetLeft = 0;
            targetRight = 0;
        }
        //왼쪽 바퀴 속도 조절
        if (targetLeft > currentLeft)
        {
            currentLeft += DUTY_SLEW_PER_TICK;
            if (currentLeft > targetLeft)
            {
                currentLeft = targetLeft;
            }
        }
        else if (targetLeft < currentLeft)
        {
            currentLeft -= DUTY_SLEW_PER_TICK;
            if (currentLeft < targetLeft)
            {
                currentLeft = targetLeft;
            }
        }
        else
        {
            currentLeft = targetLeft;
        }
        //오른쪽 바퀴 속도 조절
        if (targetRight > currentRight)
        {
            currentRight += DUTY_SLEW_PER_TICK;
            if (currentRight > targetRight)
            {
                currentRight = targetRight;
            }
        }
        else if (targetRight < currentRight)
        {
            currentRight -= DUTY_SLEW_PER_TICK;
            if (currentRight < targetRight)
            {
                currentRight = targetRight;
            }
        }
        else
        {
            currentRight = targetRight;
        }
        //방향 조절
        currentLeftDir = targetLeftDir;
        currentRightDir = targetRightDir;
        //모터 구동
        if (currentLeft == 0)
        {
            Motor_stopChA();
        }
        else
        {
            Motor_movChA_PWM(currentLeft, currentLeftDir);
        }

        if (currentRight == 0)
        {
            Motor_stopChB();
        }
        else
        {
            Motor_movChB_PWM(currentRight, currentRightDir);
        }
    }
}

void AppDrive_Init(void)
{
    if (g_driveTaskHandle != NULL)
    {
        return;
    }

    Motor_Init();
    Motor_stopChA();
    Motor_stopChB();

    BaseType_t ok = xTaskCreate(task_motor_control,
                                "drive",
                                DRIVE_TASK_STACK_WORDS,
                                NULL,
                                DRIVE_TASK_PRIORITY,
                                &g_driveTaskHandle);
    configASSERT(ok == pdPASS);
}
