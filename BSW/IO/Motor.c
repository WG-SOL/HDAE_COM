#include <gpt12.h>
#include <Ifx_reg.h>
#include <Ifx_Types.h>
#include <IfxGpt12.h>
#include <IfxPort.h>
#include "Motor.h"
#include "Bsp.h"
#include "isr_priority.h"
#include "gtm_atom_pwm.h"

// --- 💡 여기가 추가된 부분입니다 ---
// 현재 모터 상태를 저장할 전역 변수
volatile int g_current_motor_duty = 0;      // 속도 (0~100)
volatile int g_current_motor_direction = 0; // 방향 (0: 정방향, 1: 역방향)
// ------------------------------------

void Motor_Init(void)
{
    MODULE_P10.IOCR0.B.PC1 = 0x10;  // PWM A DIR
    MODULE_P02.IOCR4.B.PC7 = 0x10;  // PWM A Break

    MODULE_P10.IOCR0.B.PC2 = 0x10;  // PWM B DIR
    MODULE_P02.IOCR4.B.PC6 = 0x10;  // PWM B Break

    // Init GTM for PWM generation
    GtmAtomPwm_Init();

    // Set duty 0
    GtmAtomPwmA_SetDutyCycle(0);
    GtmAtomPwmB_SetDutyCycle(0);
    MODULE_P02.OUT.B.P7 = 1;
    MODULE_P02.OUT.B.P6 = 1;
}

///* 1: 정방향, 2: 역방향 */
void Motor_movChA(int dir)
{
    if(dir)
    {
        MODULE_P10.OUT.B.P1 = 1; /* 모터 회전 방향 (1: 앞, 0: 뒤) */
    }
    else {
        MODULE_P10.OUT.B.P1 = 0; /* 모터 회전 방향 (1: 앞, 0: 뒤) */
    }
    MODULE_P02.OUT.B.P7 = 0;   /* 모터 Brake 해제 (1: 정지, 0: PWM-A에 따라 동작) */
    GtmAtomPwm_SetDutyCycle(1000); /* 100% PWM duty  */
}

void Motor_stopChA(void)
{
    MODULE_P02.OUT.B.P7 = 1;   /* 모터 Brake 신호 인가 (1: 정지, 0: PWM-A에 따라 동작) */
}


///* 1: 정방향, 0: 역방향 */
void Motor_movChA_PWM(int duty, int dir)
{
//    GtmAtomPwm_SetDutyCycle(duty);
    GtmAtomPwmA_SetDutyCycle(duty*4);
    if(dir)
    {
        MODULE_P10.OUT.B.P1 = 1; /* 모터 회전 방향 (1: 앞, 0: 뒤) */
    }
    else {
        MODULE_P10.OUT.B.P1 = 0; /* 모터 회전 방향 (1: 앞, 0: 뒤) */
    }

    MODULE_P02.OUT.B.P7 = 0;   /* 모터 Brake 해제 (1: 정지, 0: PWM-A에 따라 동작) */
}

///* 1: 정방향, 2: 역방향 */
void Motor_movChB(int dir)
{
    if(dir)
    {
        MODULE_P10.OUT.B.P2 = 1; /* 모터 회전 방향 (1: 앞, 0: 뒤) */
    }
    else {
        MODULE_P10.OUT.B.P2 = 0; /* 모터 회전 방향 (1: 앞, 0: 뒤) */
    }
    MODULE_P02.OUT.B.P6 = 0;   /* 모터 Brake 해제 (1: 정지, 0: PWM-A에 따라 동작) */
    GtmAtomPwm_SetDutyCycle(1000); /* 100% PWM duty  */
}

void Motor_stopChB(void)
{
    MODULE_P02.OUT.B.P6 = 1;   /* 모터 Brake 신호 인가 (1: 정지, 0: PWM-A에 따라 동작) */
}


///* 1: 정방향, 0: 역방향 */
void Motor_movChB_PWM(int duty, int dir)
{
    // --- 💡 여기가 추가된 부분입니다 ---
    // 전역 변수에 현재 상태를 기록
    g_current_motor_duty = duty;
    g_current_motor_direction = dir;
    // ------------------------------------

//    GtmAtomPwm_SetDutyCycle(duty);
    GtmAtomPwmB_SetDutyCycle(duty*4);

    if(dir)
    {
        MODULE_P10.OUT.B.P2 = 1; /* 모터 회전 방향 (1: 앞, 0: 뒤) */
    }
    else {
        MODULE_P10.OUT.B.P2 = 0; /* 모터 회전 방향 (1: 앞, 0: 뒤) */
    }

    MODULE_P02.OUT.B.P6 = 0;   /* 모터 Brake 해제 (1: 정지, 0: PWM-A에 따라 동작) */
}

// --- 💡 여기가 추가된 부분입니다 ---

// 현재 설정된 모터 속도(Duty) 값을 반환하는 함수
int get_current_motor_duty(void)
{
    return g_current_motor_duty;
}

// 현재 설정된 모터 방향 값을 반환하는 함수
int get_current_motor_direction(void)
{
    return g_current_motor_direction;
}
// ------------------------------------

