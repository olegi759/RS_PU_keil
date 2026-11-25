#ifndef __user_timer_H__
#define __user_timer_H__

#include "main.h"
#include <stdbool.h>

typedef struct user_timer_t
{
    uint32_t Timestamp;

    volatile bool isSingleShot;
    volatile bool isActive;
}user_timer_t;


void initTimer(user_timer_t* p);
void startSingleShotTimer(user_timer_t* p);
void startTimer(user_timer_t* p);
void stopTimer(user_timer_t* p);
bool isTimerExpired(user_timer_t* p, uint32_t timeout);


#endif 
