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
#include "hardware/adc.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include "hc06.h"

//PINs
const int BTN_PIN_SW = 12;
const int BTN_PIN_1 = 4;
const int BTN_PIN_2 = 5;
const int BTN_PIN_3 = 6;
const int BTN_PIN_ENTER = 10;
const int BTN_PIN_UP = 11;
const int BTN_PIN_RIGHT = 12;
const int BTN_PIN_DOWN = 13;
const int BTN_PIN_LEFT = 14;
const int BTN_PIN_ON_OFF = 7;

const int X_PIN = 27;
const int Y_PIN = 28;

const int ENCA_PIN = 12;
const int ENCB_PIN = 13;

//Filas, Semaforos e structs
QueueHandle_t xQueueMouse;
QueueHandle_t xQueueBTN;
QueueHandle_t xQueueLetra;
QueueHandle_t xQueueMacaco;

typedef struct mouse {
    int axis; 
    int val;
} mouse_t;

typedef struct macaco {
    int lista; //azul= 1,verde = 2,roxo = 3 ou amarelo = 4
    int i; //Indice
} macaco_t;

typedef struct button_click {
    int button; //azul= 1,verde = 2,roxo = 3 ou amarelo = 4
    absolute_time_t time; //Indice
} button_click_t;

//Listas de Macacos do jogo e respectivos atalhos
const char * azul[] = {"Macaco Dardo", "Macaco Bumerangue", "Bombardeiro", "Cospe Tachinha", "Macaco de Gelo", "Cospe Cola"};
const char a_azul[] = {'Q','W','E','R','T','Y'};

const char * verde[] = {"Macaco Atirador", "Macaco Sub", "Macaco Bucaneiro", "Macaco Ás", "Helicóptero", "Macaco Morteiro", "Arma de Dardos"};
const char a_verde[] = {'Z','X','C','V','B','N','M'};

const char * roxo[] = {"Macaco Mago", "Super Macaco", "Macaco Ninja", "Alquimista", "Druida"};
const char a_roxo[] = {'A','S','D','F','G'};

const char * amarelo[] = {"Heroi", "Fazenda", "Usina de Espinhos", "Vila Macaco", "Macaco Engenheiro", "Domador de Feras", "Fazendeiro"};
const char a_amarelo[] = {'U','H','J','K','L','I','O'};

const char a_melhorias[] = {',','.','/'};

//Configuração do Encoder
const int8_t state_table[] = {
        0, -1,  1,  0,
        1,  0,  0, -1,
        -1,  0,  0,  1,
        0,  1, -1,  0
    };

//IRQS________________________________________________________________________________________________________________________________
void btn_callback(uint gpio, uint32_t events) {
    button_click_t btn;
    // 0--> SW, 1-->BTN1, 2-->BTN2, 3-->BTN3,4-->ENTER, 5-->UP, 6-->RIGHT, 7-->DOWN, 8-->LEFT, 9-->ON_OFF
    if (events == (0x04)) { 
        if (gpio == BTN_PIN_SW){
            btn.button = 0;
            btn.time = get_absolute_time();
        }
        else if (gpio == BTN_PIN_1){
            btn.button = 1;
            btn.time = get_absolute_time();
        }
        else if (gpio == BTN_PIN_2){
            btn.button = 2;
            btn.time = get_absolute_time();
        }
        else if (gpio == BTN_PIN_3){
            btn.button = 3;
            btn.time = get_absolute_time();
        }
        else if (gpio == BTN_PIN_ENTER){
            btn.button = 4;
            btn.time = get_absolute_time();
        }
        else if (gpio == BTN_PIN_UP){
            btn.button = 5;
            btn.time = get_absolute_time();
        }
        else if (gpio == BTN_PIN_RIGHT){
            btn.button = 6;
            btn.time = get_absolute_time();
        }
        else if (gpio == BTN_PIN_DOWN){
            btn.button = 7;
            btn.time = get_absolute_time();
        }
        else if (gpio == BTN_PIN_LEFT){
            btn.button = 8;
            btn.time = get_absolute_time();
        }
        else if (gpio == BTN_PIN_ON_OFF){
            btn.button = 9;
            btn.time = get_absolute_time();
        }       
    }

    xQueueSendFromISR(xQueueBTN, &btn, 1);
}

// TASKS_______________________________________________________________________________________________________________________________
void hc06_task(void *p) {
    uart_init(HC06_UART_ID, HC06_BAUD_RATE);
    gpio_set_function(HC06_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(HC06_RX_PIN, GPIO_FUNC_UART);
    hc06_init("aps2_legal", "1234");

    char letra;
    struct mouse mouse_data;
    while (true) {
        if (xQueueReceive(xQueueLetra, &letra, 1)) {
            uart_puts(HC06_UART_ID, letra);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        if (xQueueReceive(xQueueMouse, &mouse_data, 1)) {
            uart_puts(HC06_UART_ID, mouse_data.axis);
            uart_puts(HC06_UART_ID, mouse_data.val);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

void botao_task(void *p) {
    //printf("Botao task\n");
    gpio_init(BTN_PIN_SW);
    gpio_set_dir(BTN_PIN_SW, GPIO_IN);
    gpio_pull_up(BTN_PIN_SW);

    gpio_init(BTN_PIN_1);
    gpio_set_dir(BTN_PIN_1, GPIO_IN);
    gpio_pull_up(BTN_PIN_1);

    gpio_init(BTN_PIN_2);
    gpio_set_dir(BTN_PIN_2, GPIO_IN);
    gpio_pull_up(BTN_PIN_2);

    gpio_init(BTN_PIN_3);
    gpio_set_dir(BTN_PIN_3, GPIO_IN);
    gpio_pull_up(BTN_PIN_3);

    gpio_init(BTN_PIN_ENTER);
    gpio_set_dir(BTN_PIN_ENTER, GPIO_IN);
    gpio_pull_up(BTN_PIN_ENTER);

    gpio_init(BTN_PIN_UP);
    gpio_set_dir(BTN_PIN_UP, GPIO_IN);
    gpio_pull_up(BTN_PIN_UP);

    gpio_init(BTN_PIN_RIGHT);
    gpio_set_dir(BTN_PIN_RIGHT, GPIO_IN);
    gpio_pull_up(BTN_PIN_RIGHT);

    gpio_init(BTN_PIN_DOWN);
    gpio_set_dir(BTN_PIN_DOWN, GPIO_IN);
    gpio_pull_up(BTN_PIN_DOWN);

    gpio_init(BTN_PIN_LEFT);
    gpio_set_dir(BTN_PIN_LEFT, GPIO_IN);
    gpio_pull_up(BTN_PIN_LEFT);

    gpio_init(BTN_PIN_ON_OFF);
    gpio_set_dir(BTN_PIN_ON_OFF, GPIO_IN);
    gpio_pull_up(BTN_PIN_ON_OFF);

    gpio_set_irq_enabled_with_callback(BTN_PIN_1, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    gpio_set_irq_enabled(BTN_PIN_SW, GPIO_IRQ_EDGE_FALL,true);
    gpio_set_irq_enabled(BTN_PIN_2, GPIO_IRQ_EDGE_FALL,true);
    gpio_set_irq_enabled(BTN_PIN_3, GPIO_IRQ_EDGE_FALL,true);
    gpio_set_irq_enabled(BTN_PIN_ENTER, GPIO_IRQ_EDGE_FALL,true);
    gpio_set_irq_enabled(BTN_PIN_UP, GPIO_IRQ_EDGE_FALL,true);
    gpio_set_irq_enabled(BTN_PIN_RIGHT, GPIO_IRQ_EDGE_FALL,true);
    gpio_set_irq_enabled(BTN_PIN_DOWN, GPIO_IRQ_EDGE_FALL,true);
    gpio_set_irq_enabled(BTN_PIN_LEFT, GPIO_IRQ_EDGE_FALL,true);
    gpio_set_irq_enabled(BTN_PIN_ON_OFF, GPIO_IRQ_EDGE_FALL,true);


    button_click_t btn;
    
    //printf("CONFIGURADO\n\n");
    while (1) {
        //printf("xQueueBTN: ");
        absolute_time_t start_time = get_absolute_time();
        uint32_t start_time_ms = to_ms_since_boot(start_time);
        if (xQueueReceiveFromISR(xQueueBTN, &btn, 0)) {
            uint32_t pressed_time = to_ms_since_boot(btn.time);
            uint32_t delta_time = (start_time_ms - pressed_time);
            //printf("%d\n", (pressed_time - start_time_ms));
            if (delta_time < 7)
            {
                printf("OK\n");
                printf("Botao pressionado: %d\n", btn);
                printf("Delta: %d\n", delta_time);
            }
            else
            {
                printf("Debounced\n");
                printf("Delta: %d\n", delta_time);
            }
        }
        //printf("W\n");
        vTaskDelay(pdMS_TO_TICKS(10));
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

        struct mouse mouse_data_x = {0,(int)x};
        xQueueSend(xQueueMouse, &mouse_data_x, 1);
        

        //Y
        adc_select_input(2); // Select ADC input 2 (GPIO28)
        int y = adc_read();
        //printf("Y: %d V\n", y);
        //Calcula a deadzone
        y = (y-2047)/8;
        if (y <=zone_limit && y >= -1*(zone_limit)) {
            y = 0;
        }
        
        struct mouse mouse_data_y = {1,(int)y};
        xQueueSend(xQueueMouse, &mouse_data_y, 1);

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void seletor_task(void *p) {

    uint8_t enc_state = 0; // Current state of the encoder
    int8_t last_encoded = 0; // Last encoded state
    int8_t encoded;
    int sum;
    int last_sum = 0; // Last non-zero sum to filter out noise
    int debounce_counter = 0; // Debounce counter

    // Inicialização do Encoder
    gpio_init(ENCA_PIN);
    gpio_init(ENCB_PIN);
    gpio_set_dir(ENCA_PIN, GPIO_IN);
    gpio_set_dir(ENCB_PIN, GPIO_IN);
    gpio_pull_up(ENCA_PIN);
    gpio_pull_up(ENCB_PIN);

    last_encoded = (gpio_get(ENCA_PIN) << 1) | gpio_get(ENCB_PIN);

    printf("Inicializando Driver\n");
    ssd1306_init();

    printf("Inicializando GLX\n");
    ssd1306_t disp;
    gfx_init(&disp, 128, 32);

    char str[100];
    macaco_t mcaco;
    while (1) {
        encoded = (gpio_get(ENCA_PIN) << 1) | gpio_get(ENCB_PIN);
        enc_state = (enc_state << 2) | encoded;
        sum = state_table[enc_state & 0x0f];

         if (sum != 0) {
            if (sum == last_sum) {
                if (++debounce_counter > 1) {  // Check if the same movement is read consecutively
                    if (sum == 1) {
                        printf("RIGHT\n");
                    } else if (sum == -1) {
                        printf("LEFT\n");
                    }
                    debounce_counter = 0;  // Reset the counter after confirming the direction
                }
            } else {
                debounce_counter = 0;  // Reset the counter if the direction changes
            }
            last_sum = sum;  // Update last_sum to the current sum
        }

        vTaskDelay(pdMS_TO_TICKS(1)); // Poll every 1 ms to improve responsiveness
        if (xQueueReceive(xQueueMacaco, &mcaco,  0)) {
            int lista;
            int indice;

            lista = mcaco.lista; //azul= 1,verde = 2,roxo = 3 ou amarelo = 4
            indice = mcaco.i;

            //Acessa o valor i da lista, conforme fornecido pela fila
            if (lista == 1)
                strcpy(str, azul[indice]);
            else if (lista == 2)
                strcpy(str, verde[indice]);
            else if (lista == 3)
                strcpy(str, roxo[indice]);
            else if (lista == 4)
                strcpy(str, amarelo[indice]);
            
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

    //printf("Start bluetooth task\n");

    //Semaforos

    //Filas
    xQueueBTN = xQueueCreate(32, sizeof(button_click_t));
    xQueueMouse = xQueueCreate(32, sizeof(mouse_t));
    xQueueLetra = xQueueCreate(32, sizeof(char));
    xQueueMacaco = xQueueCreate(32, sizeof(macaco_t));


    //Tasks
    //xTaskCreate(mouse_task, "Mouse_Task", 4095, NULL, 1, NULL);
    //xTaskCreate(hc06_task, "UART_Task 1", 4096, NULL, 1, NULL);
    //xTaskCreate(seletor_task, "Display", 4095, NULL, 1, NULL);
    xTaskCreate(botao_task, "Botao_Task", 4095, NULL, 1, NULL);


    vTaskStartScheduler();

    while (true)
        ;
}
