#ifndef BSW_IO_MOTOR_H_
#define BSW_IO_MOTOR_H_

void Motor_Init(void);

void Motor_movChA(int dir);
void Motor_stopChA(void);
void Motor_movChA_PWM(int duty, int dir);

void Motor_movChB(int dir);
void Motor_stopChB(void);
void Motor_movChB_PWM(int duty, int dir);

// --- 💡 여기가 추가된 부분입니다 ---
int get_current_motor_duty(void);
int get_current_motor_direction(void);
// ------------------------------------

#endif /* BSW_IO_MOTOR_H_ */
