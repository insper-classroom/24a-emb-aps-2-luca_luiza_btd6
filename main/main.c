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
const int BTN_PIN_SW = 2;
const int BTN_PIN_1 = 3;
const int BTN_PIN_2 = 4;
const int BTN_PIN_3 = 5;
const int BTN_PIN_ENTER = 26; //
const int BTN_PIN_UP = 12; //
const int BTN_PIN_RIGHT = 6;
const int BTN_PIN_DOWN = 7; 
const int BTN_PIN_LEFT = 8;
const int BTN_PIN_ON_OFF = 13;

const int X_PIN = 27;
const int Y_PIN = 28;

const int ENCA_PIN = 18;
const int ENCB_PIN = 19;

//Filas, Semaforos e structs
QueueHandle_t xQueueMouse;
QueueHandle_t xQueueBTN;
QueueHandle_t xQueueLetra;
QueueHandle_t xQueueMacaco;
QueueHandle_t xQueueMacacoEnter;
TaskHandle_t xHandle = NULL; // Habilita o controle das tasks por outra task

typedef struct mouse {
    int axis; 
    int val;
} mouse_t;

typedef struct macaco {
    int lista; //azul= 1,verde = 2,roxo = 3 ou amarelo = 4
    int i; //Indice
} macaco_t;

//Listas de Macacos do jogo e respectivos atalhos
const char * azul[] = {"Macaco Dardo", "Macaco Bumerangue", "Bombardeiro", "Cospe Tachinha", "Macaco de Gelo", "Cospe Cola"};
const char a_azul[] = {'Q','W','E','R','T','Y'};
const int azul_len = 6;

const char * verde[] = {"Macaco Atirador", "Macaco Sub", "Macaco Bucaneiro", "Macaco Ás", "Helicóptero", "Macaco Morteiro", "Arma de Dardos"};
const char a_verde[] = {'Z','X','C','V','B','N','M'};
const int verde_len = 7;

const char * roxo[] = {"Macaco Mago", "Super Macaco", "Macaco Ninja", "Alquimista", "Druida"};
const char a_roxo[] = {'A','S','D','F','G'};
const int roxo_len = 5;

const char * amarelo[] = {"Heroi", "Fazenda", "Usina de Espinhos", "Vila Macaco", "Macaco Engenheiro", "Domador de Feras", "Fazendeiro"};
const char a_amarelo[] = {'U','H','J','K','L','I','O'};
const int amarelo_len = 7;

//Configuração do Encoder
const int8_t state_table[] = {
        0, -1,  1,  0,
        1,  0,  0, -1,
        -1,  0,  0,  1,
        0,  1, -1,  0
    };

//IRQS________________________________________________________________________________________________________________________________

void btn_callback(uint gpio, uint32_t events) {
    uint16_t btn;
    // 0--> SW, 1-->BTN1, 2-->BTN2, 3-->BTN3,4-->ENTER, 5-->UP, 6-->RIGHT, 7-->DOWN, 8-->LEFT, 9-->ON_OFF
    if (events == (0x08)) { 
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
        else if (gpio == BTN_PIN_ON_OFF){
            btn = 9;
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

void seletor_task(void *p) {

    //Comfiguração do Encoder
    uint8_t enc_state = 0; // Current state of the encoder
    int8_t last_encoded = 0; // Last encoded state
    int8_t encoded;
    int sum = 0;
    int last_sum = 0; // Last non-zero sum to filter out noise
    int debounce_counter = 0; // Debounce counter
    int count = 0;

    // Inicialização do Encoder
    gpio_init(ENCA_PIN);
    gpio_init(ENCB_PIN);
    gpio_set_dir(ENCA_PIN, GPIO_IN);
    gpio_set_dir(ENCB_PIN, GPIO_IN);
    gpio_pull_up(ENCA_PIN);
    gpio_pull_up(ENCB_PIN);
    last_encoded = (gpio_get(ENCA_PIN) << 1) | gpio_get(ENCB_PIN);

    // Inicialização do Display
    ssd1306_init();
    ssd1306_t disp;
    gfx_init(&disp, 128, 32);

    // Inicialização de Listas que serão utilizadas no display
    char str[100];
    macaco_t mcaco; // Struct que contém a lista de macacos
    int lista_len; // Tamanho da lista
    const char **lista; // Lista de macacos
    int i; // Indice da lista
    int cor;



    if (xQueueReceive(xQueueMacaco, &mcaco,  0)) {
        cor = mcaco.lista; //azul= 1,verde = 2,roxo = 3 ou amarelo = 4
        // Carrega a lista no display, dependendo do botão pressionado
        if (cor == 1) { //Azul
            lista = azul;
            lista_len = azul_len;
        } else if (cor == 2) { // Verde
            lista = verde;
            lista_len = verde_len;
        } else if (cor == 3) { // Roxo
            lista = roxo;
            lista_len = roxo_len;
        } else if (cor == 4) { // Amarelo
            lista = amarelo;
            lista_len = amarelo_len;
        }
    }

    // Use the lista and lista_len variables in your code
    while (1) {
        encoded = (gpio_get(ENCA_PIN) << 1) | gpio_get(ENCB_PIN);
        enc_state = (enc_state << 2) | encoded;
        sum = state_table[enc_state & 0x0f];

         if (sum != 0) {
            if (sum == last_sum) {
                if (++debounce_counter > 1) {  // Check if the same movement is read consecutively
                    if (sum == 1) {
                        count++;
                        i = count%6;
                        sprintf(str, "%s", azul[i]);
                        gfx_clear_buffer(&disp);
                        gfx_draw_string(&disp, 0, 0, 1, str);
                        gfx_show(&disp);
                    } 
                    else if (sum == -1) {
                        count--;
                        i = count % 6;
                        if (i < 0) { // Caso o indice seja menor que zero, volta para o fim da lista
                            i = azul_len - 1;
                            count = azul_len - 1;
                        }
                        sprintf(str, "%s", azul[i]);
                        gfx_clear_buffer(&disp);
                        gfx_draw_string(&disp, 0, 0, 1, str);
                        gfx_show(&disp);
                    }
                    debounce_counter = 0;  // Reset the counter after confirming the direction
                    
                    //Caso o botão enter seja pressionado, o macaco selecionado é enviado para a fila
                    char letra;
                    if (xQueueReceive(xQueueLetra, &letra, 0)) {
//                        if (letra = ','){
                            char macaco_selecionado = a_azul[i];

                            //xQueueSend(xQueueLetra, macaco_selecionado, 1);
//                        }
                    }
                }
            } else {
                debounce_counter = 0;  // Reset the counter if the direction changes
            }
            last_sum = sum;  // Update last_sum to the current sum
        }

        vTaskDelay(pdMS_TO_TICKS(1)); // Poll every 1 ms to improve responsiveness


    }
}

void botao_task(void *p) {
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

    gpio_set_irq_enabled_with_callback(BTN_PIN_1, GPIO_IRQ_EDGE_RISE, true, &btn_callback);
    gpio_set_irq_enabled(BTN_PIN_SW, GPIO_IRQ_EDGE_RISE,true);
    gpio_set_irq_enabled(BTN_PIN_2, GPIO_IRQ_EDGE_RISE,true);
    gpio_set_irq_enabled(BTN_PIN_3, GPIO_IRQ_EDGE_RISE,true);
    gpio_set_irq_enabled(BTN_PIN_ENTER, GPIO_IRQ_EDGE_RISE,true);
    gpio_set_irq_enabled(BTN_PIN_UP, GPIO_IRQ_EDGE_RISE,true);
    gpio_set_irq_enabled(BTN_PIN_RIGHT, GPIO_IRQ_EDGE_RISE,true);
    gpio_set_irq_enabled(BTN_PIN_DOWN, GPIO_IRQ_EDGE_RISE,true);
    gpio_set_irq_enabled(BTN_PIN_LEFT, GPIO_IRQ_EDGE_RISE,true);
    gpio_set_irq_enabled(BTN_PIN_ON_OFF, GPIO_IRQ_EDGE_RISE,true);


    uint16_t btn;
    int liga_desliga = 0;
    macaco_t mcaco;
    while (1) {
        //printf("xQueueBTN: ");
        if (xQueueReceiveFromISR(xQueueBTN, &btn, 1)) {
            //printf("Botao pressionado: %d\n", btn);
            // 0--> SW, 1-->BTN1, 2-->BTN2, 3-->BTN3,4-->ENTER, 5-->UP, 6-->RIGHT, 7-->DOWN, 8-->LEFT, 9-->ON_OFF
            //const char a_melhorias[] = {',','.','/'};

            if (btn == 0) {
                char letra = '|'; //Clique esquerdo do mouse
                xQueueSend(xQueueLetra, &letra, 1);
                //printf("Clique esquerdo do mouse\n");
            }
            else if (btn == 1) {
                char letra = ',';
                xQueueSend(xQueueLetra, &letra, 1);
                //printf("Botão 1\n");
            }
            else if (btn == 2) {
                char letra = '.';
                xQueueSend(xQueueLetra, &letra, 1);
                //printf("Botão 2\n");
            }
            else if (btn == 3) {
                char letra = '/';
                xQueueSend(xQueueLetra, &letra, 1);
                //printf("Botão 3\n");
            }
            else if (btn == 4) {
                int enter = 1;
                xQueueSend(xQueueMacacoEnter, &enter, 1);
                //printf("Enter\n");
            }
            else if (btn == 5) {
                mcaco.lista = 1;
                xQueueSend(xQueueMacaco, &mcaco, 1);
                //printf("UP\n");
            }
            else if (btn == 6) {
                mcaco.lista = 2;
                xQueueSend(xQueueMacaco, &mcaco, 1);
                //printf("RIGHT\n");
            }
            else if (btn == 7) {    
                mcaco.lista = 3;
                xQueueSend(xQueueMacaco, &mcaco, 1);
                //printf("DOWN\n");
            }
            else if (btn == 8) {
                mcaco.lista = 4;
                xQueueSend(xQueueMacaco, &mcaco, 1);
                //printf("LEFT\n");
            }
            else if (btn == 9) {
                if (xHandle == NULL){
                    xTaskCreate(hc06_task, "UART_Task 1", 4096, NULL, 1, &xHandle);
                    xTaskCreate(seletor_task, "Display", 4095, NULL, 1, &xHandle);   
                }
                else {
                    vTaskDelete(xHandle);
                    xHandle = NULL;
                }
            }
        }
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



int main() {
    stdio_init_all();

    //printf("Start bluetooth task\n");

    //Semaforos

    //Filas
    xQueueBTN = xQueueCreate(32, sizeof(uint16_t));
    xQueueMouse = xQueueCreate(32, sizeof(mouse_t));
    xQueueLetra = xQueueCreate(32, sizeof(char));
    xQueueMacaco = xQueueCreate(32, sizeof(macaco_t));
    xQueueMacacoEnter = xQueueCreate(32, sizeof(int));


    //Tasks
    //xTaskCreate(mouse_task, "Mouse_Task", 4095, NULL, 1, NULL);
    //xTaskCreate(hc06_task, "UART_Task 1", 4096, NULL, 1, NULL);
    //xTaskCreate(seletor_task, "Display", 4095, NULL, 1, NULL);
    xTaskCreate(botao_task, "Botao_Task", 4095, NULL, 1, NULL);


    vTaskStartScheduler();

    while (true)
        ;
}
