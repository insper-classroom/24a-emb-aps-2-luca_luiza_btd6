/*
 * LED blink with FreeRTOS
 */
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>
#include <string.h>
#include "ssd1306.h"
#include "gfx.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include "hc06.h"

//PINs
BTN_PIN_SW = 6;

X_PIN = 27;
Y_PIN = 28;

//Filas, Semaforos e structs
QueueHandle_t xQueueMouse;
QueueHandle_t xQueueBTN;
QueueHandle_t xQueueLetra;
QueueHandle_t xQueueMacaco;

typedef struct mouse {
    int x; //pos_x
    int y; //pos_y
    int sw; //click_joystick
} mouse_t;

typedef struct macaco {
    int lista; //azul= 1,verde = 2,roxo = 3 ou amarelo = 4
    int i; //Indice
} macaco_t;

//Listas de Macacos do jogo e respectivos atalhos
const char azul = ["Macaco Dardo", "Macaco Bumerangue", "Bombardeiro", "Cospe Tachinha", "Macaco de Gelo", "Cospe Cola"]
const char a_azul = ['Q','W','E','R','T','Y']

const char verde = ["Macaco Atirador", "Macaco Sub", "Macaco Bucaneiro", "Macaco Ás", "Helicóptero", "Macaco Morteiro", "Arma de Dardos"]
const char a_verde = ['Z','X','C','V','B','N','M']

const char roxo = ["Macaco Mago", "Super Macaco", "Macaco Ninja", "Alquimista", "Druida"]
const char a_roxo = ['A','S','D','F','G']

const char amarelo = ["Heroi", "Fazenda", "Usina de Espinhos", "Vila Macaco", "Macaco Engenheiro", "Domador de Feras", "Fazendeiro"]
const char a_amarelo = ['U','H','J','K','L','I','O']

//IRQS________________________________________________________________________________________________________________________________
void btn_callback(uint gpio, uint32_t events) {
    uint16_t btn;
    // 0--> SW, 1-->BTN1, 2-->BTN2, 3-->BTN3,4-->ENTER, 5-->UP, 6-->RIGHT, 7-->DOWN, 8-->LEFT
    if (events == 0x4) { // fall edge
        if (gpio == BTN_PIN_SW){
            btn = 0;
        }
        else if (gpio == BTN_PIN_1){
            btn = 1;
        }
        else if (gpio == BTN_PIN_2){
            btn = 2;
        }
        else if (gpio == BTN_PIN_3){
            btn = 3;
        }
        else if (gpio == BTN_PIN_ENTER){
            btn = 4;
        }
        else if (gpio == BTN_PIN_UP){
            btn = 5;
        }
        else if (gpio == BTN_PIN_RIGHT){
            btn = 6;
        }
        else if (gpio == BTN_PIN_DOWN){
            btn = 7;
        }
        else if (gpio == BTN_PIN_LEFT){
            btn = 8;
        }

        //Envia o botão pressionado
        xQueueSendFromISR(xQueueBTN, &btn, NULL);
    }
}

// TASKS_______________________________________________________________________________________________________________________________
void hc06_task(void *p) {
    uart_init(HC06_UART_ID, HC06_BAUD_RATE);
    gpio_set_function(HC06_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(HC06_RX_PIN, GPIO_FUNC_UART);
    hc06_init("aps2_legal", "1234");

    while (true) {
        uart_puts(HC06_UART_ID, "OLAAA ");
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void mouse_task(void *p){
    adc_init();
    adc_gpio_init(X_PIN); // X

    adc_init();
    adc_gpio_init(Y_PIN); //Y

    gpio_set_irq_enabled_with_callback(BTN_PIN_SW, GPIO_IRQ_EDGE_FALL, true, &btn_callback); //SW

    int zone_limit = 80;
    
    while (1) {
        //X
        adc_select_input(1); // Select ADC input 1 (GPIO27)
        int x = adc_read();
        //printf("X: %d V\n", x);
        //Calcula a deadzone
        x = (x-2047)/8;
        if (x <=zone_limit && x >= -1*(zone_limit)) {
            x = 0;
        }
        
        //Y
        adc_select_input(2); // Select ADC input 2 (GPIO28)
        int y = adc_read();
        //printf("Y: %d V\n", y);
        //Calcula a deadzone
        y = (y-2047)/8;
        if (y <=zone_limit && y >= -1*(zone_limit)) {
            y = 0;
        }

        //SW
        uint16_t btn;
        if (xQueueReceive(xQueueBTN, &btn,pdMS_TO_TICKS(100))) {
            if (btn = 0){
                int sw = 1;
            }
            else{
                int sw = 0;
            }
        }
        else{
            int sw = 0;
        }

        
        struct mouse mouse_data = {x,y,sw};
        xQueueSend(xQueueMouse, &mouse_data, portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void oled_task(void *p) {
    printf("Inicializando Driver\n");
    ssd1306_init();

    printf("Inicializando GLX\n");
    ssd1306_t disp;
    gfx_init(&disp, 128, 32);

    char str[100];
    macaco mcaco;
    while (1) {
        if (xQueueReceive(xQueueMacaco, &mcaco,  0)) {
            lista = mcaco.lista; //azul= 1,verde = 2,roxo = 3 ou amarelo = 4
            indice = mcaco.i;

            //Acessa o valor i da lista, conforme fornecido pela fila
            if (lista == 1)
                str = azul[indice];
            else if (lista == 2)
                str = verde[indice];
            else if (lista == 3)
                str = roxo[indice];
            else if (lista == 4)
                str = amarelo[indice];
            
                gfx_clear_buffer(&disp);
                gfx_draw_string(&disp, 0, 0, 2, str);
                vTaskDelay(pdMS_TO_TICKS(50));
                gfx_show(&disp);
            } 
        else {
            gfx_clear_buffer(&disp);
            gfx_draw_string(&disp, 0, 0, 4, "BTD6!!!");
            gfx_show(&disp);
        }
    }
}

int main() {
    stdio_init_all();

    printf("Start bluetooth task\n");

    //Semaforos

    //Filas
    xQueueBTN = xQueueCreate(32, sizeof(uint16_t));
    xQueueMouse = xQueueCreate(32, sizeof(mouse_t));
    xQueueLetra = xQueueCreate(32, sizeof(char));
    xQueueMacaco = xQueueCreate(32, sizeof(macaco_t));


    //Tasks
    xTaskCreate(mouse_task, "Mouse_Task", 256, NULL, 1, NULL);
    xTaskCreate(hc06_task, "UART_Task 1", 4096, NULL, 1, NULL);
    xTaskCreate(oled_task, "Display", 4095, NULL, 1, NULL);


    vTaskStartScheduler();

    while (true)
        ;
}
