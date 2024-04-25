#ifndef hc05_H_
#define hc05_H_

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include <string.h>

#include "pico/stdlib.h"
#include <stdio.h>

#define hc05_UART_ID uart0
#define hc05_BAUD_RATE 9600
#define hc05_RX_PIN 0
#define hc05_TX_PIN 1
#define hc05_PIN 13

bool hc05_check_connection();
bool hc05_set_name(char name[]);
bool hc05_set_pin(char pin[]);
bool hc05_set_at_mode(int on);
bool hc05_init(char name[], char pin[]);


#endif // hc05_H_