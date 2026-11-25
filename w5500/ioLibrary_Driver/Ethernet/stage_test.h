#ifndef STAGE_TESTS_H
#define STAGE_TESTS_H

#include <stdint.h>

/* Возврат 0 = успех, иначе — номер упавшей проверки. */
int stage3_test(void);
int stage4_test_basic(void);
int stage4_test_isolation(void);

void tcp_echo_open_stage(uint8_t vms, uint16_t port);
void tcp_echo_poll_stage(uint8_t vms);

#endif