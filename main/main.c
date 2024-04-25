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
#include "hc05.h"

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
QueueHandle_t xQueueClasse;
QueueHandle_t xQueueEnter; // 
TaskHandle_t xHandle = NULL; // Habilita o controle das tasks por outra task

typedef struct mouse {
    int axis; 
    int val;
} mouse_t;


//Listas de Macacos do jogo e respectivos atalhos
const char * azul[] = {"Macaco Dardo", "Macaco Bumerangue", "Bombardeiro", "Cospe Tachinha", "Macaco de Gelo", "Cospe Cola"};
const char a_azul[] = {'Q','W','E','R','T','Y'};
const int azul_len = 6;

const char * verde[] = {"Macaco Atirador", "Macaco Sub", "Macaco Bucaneiro", "Macaco As", "Helicoptero", "Macaco Morteiro", "Arma de Dardos"};
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


//FUNCOES________________________________________________________________________________________________________________________________
void mouse_write_package(mouse_t data) {
    int val = data.val;
    int msb = val >> 8;
    int lsb = val & 0xFF ;
    
    uart_putc_raw(hc05_UART_ID, data.axis); 
    uart_putc_raw(hc05_UART_ID, lsb);
    uart_putc_raw(hc05_UART_ID, msb); 
    uart_putc_raw(hc05_UART_ID, -1); 
}

//IRQS________________________________________________________________________________________________________________________________

void btn_callback(uint gpio, uint32_t events) {
    static TickType_t last_interrupt_time = 0;
    uint16_t btn;
    // 0--> SW, 1-->BTN1, 2-->BTN2, 3-->BTN3,4-->ENTER, 5-->UP, 6-->RIGHT, 7-->DOWN, 8-->LEFT, 9-->ON_OFF
    if (events == (0x08)) { 
        TickType_t interrupt_time = xTaskGetTickCountFromISR();
        if ((interrupt_time - last_interrupt_time) > pdMS_TO_TICKS(50)) {
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
            // else if (gpio == BTN_PIN_ON_OFF){
            //     btn = 9;
            // }       
            xQueueSendFromISR(xQueueBTN, &btn, 1);
        }
        last_interrupt_time = interrupt_time;
    }
}

// TASKS_______________________________________________________________________________________________________________________________
void seletor_task(void *p) {
    //Configuração do Encoder
    uint8_t enc_state = 0; // Current state of the encoder
    //int8_t last_encoded = 0; // Last encoded state
    //int8_t encoded;

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
    //last_encoded = (gpio_get(ENCA_PIN) << 1) | gpio_get(ENCB_PIN);

    // Inicialização do Display
    ssd1306_init();
    ssd1306_t disp;
    gfx_init(&disp, 128, 32);

    // Inicialização de Listas que serão utilizadas no display
    char str[100]; // String que será exibida no display
    int lista_len = azul_len; // Tamanho da lista
    const char **lista = azul; // Lista de macacos
    const char *a_lista = a_azul; // Lista de atalhos
    char l; // Letra do indice atual
    int i; // Indice da lista
    int cor; // classe
    int enter; // enter



    // Use the lista and lista_len variables in your code
    while (1) {

        if (xQueueReceive(xQueueClasse, &cor,  0)) {
            // Carrega a lista no display, dependendo do botão pressionado
            if (cor == 1) { //Azul
                lista = azul;
                lista_len = azul_len;
                a_lista = a_azul;
            } else if (cor == 2) { // Verde
                lista = verde;
                lista_len = verde_len;
                a_lista = a_verde;
            } else if (cor == 3) { // Roxo
                lista = roxo;
                lista_len = roxo_len;
                a_lista = a_roxo;
            } else if (cor == 4) { // Amarelo
                lista = amarelo;
                lista_len = amarelo_len;
                a_lista = a_amarelo;
            }
            l = a_lista[0];
            // printf("%c \n", l); //DEBUG
            sprintf(str, "%s", lista[0]);
            gfx_clear_buffer(&disp);
            gfx_draw_string(&disp, 0, 0, 1.5, str);
            gfx_show(&disp);
        }

        int8_t encoded = (gpio_get(ENCA_PIN) << 1) | gpio_get(ENCB_PIN);
        enc_state = (enc_state << 2) | encoded;
        int sum = state_table[enc_state & 0x0f];

        if (sum != 0) {
            if (sum == last_sum) {
                if (++debounce_counter > 1) {  // Check if the same movement is read consecutively
                    if (sum == 1) {
                        count++;
                        i = count%lista_len;
                        l = a_lista[i];
                        // printf("%c \n", l); //Debug
                        sprintf(str, "%s", lista[i]);
                        gfx_clear_buffer(&disp);
                        gfx_draw_string(&disp, 0, 0, 1.5, str);
                        gfx_show(&disp);
                    } 
                    else if (sum == -1) {
                        count--;
                        i = count % lista_len;
                        if (i < 0) { // Caso o indice seja menor que zero, volta para o fim da lista
                            i = azul_len - 1;
                            count = azul_len - 1;
                        }
                        l = a_lista[i];
                        // printf("%c \n", l); //Debug
                        sprintf(str, "%s", lista[i]);
                        gfx_clear_buffer(&disp);
                        gfx_draw_string(&disp, 0, 0, 1.5, str);
                        gfx_show(&disp);
                    }
                    debounce_counter = 0;  // Reset the counter after confirming the direction
                    
                    //Caso o botão enter seja pressionado, o macaco selecionado é enviado para a fila
                    // If queuebotaoenter, envia a letra atual
                }
            } 
            else {
                debounce_counter = 0;  // Reset the counter if the direction changes
            }
            last_sum = sum;  // Update last_sum to the current sum
        }

        if (xQueueReceive(xQueueEnter, &enter, 0)) {
            char letra = l;
            xQueueSend(xQueueLetra, &letra, 1);
            // printf("Macaco selecionado: %c\n", letra); // Debug
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
    int cor;
    while (1) {
        //printf("xQueueBTN: ");
        if (xQueueReceiveFromISR(xQueueBTN, &btn, 1)) {
            //printf("Botao pressionado: %d\n", btn);
            // 0--> SW, 1-->BTN1, 2-->BTN2, 3-->BTN3,4-->ENTER, 5-->UP, 6-->RIGHT, 7-->DOWN, 8-->LEFT, 9-->ON_OFF
            //const char a_melhorias[] = {',','.','/'};

            if (btn == 0) {
                char letra = '|'; //Clique esquerdo do mouse
                xQueueSend(xQueueLetra, &letra, 1);
                // printf("Clique esquerdo do mouse\n"); // Debug
            }
            else if (btn == 1) {
                char letra = ',';
                xQueueSend(xQueueLetra, &letra, 1);
                // printf("Botão 1\n"); // Debug
            }
            else if (btn == 2) {
                char letra = '.';
                xQueueSend(xQueueLetra, &letra, 1);
                // printf("Botão 2\n"); // Debug
            }
            else if (btn == 3) {
                char letra = '/';
                xQueueSend(xQueueLetra, &letra, 1);
                // printf("Botão 3\n"); // Debug
            }
            else if (btn == 4) {
                int enter = 1;
                xQueueSend(xQueueEnter, &enter, 1);
                // printf("Enter\n"); // Debug
            }
            else if (btn == 5) {
                cor = 1;
                xQueueSend(xQueueClasse, &cor, 1);
                // printf("UP\n"); // Debug
            }
            else if (btn == 6) {
                cor = 2;
                xQueueSend(xQueueClasse, &cor, 1);
                // printf("RIGHT\n"); // Debug
            }
            else if (btn == 7) {    
                cor = 3;
                xQueueSend(xQueueClasse, &cor, 1);
                // printf("DOWN\n"); // Debug
            }
            else if (btn == 8) {
                cor = 4;
                xQueueSend(xQueueClasse, &cor, 1);
                // printf("LEFT\n"); // Debug
            }
            // else if (btn == 9) {
            //     if (xHandle == NULL){
            //         //xTaskCreate(hc06_task, "UART_Task 1", 4096, NULL, 1, &xHandle);
            //         xTaskCreate(seletor_task, "Display", 4095, NULL, 1, &xHandle);   
            //     }
            //     else {
            //         vTaskDelete(xHandle);
            //         xHandle = NULL;
            //     }
            // }
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void mouse_task(void *p){
    adc_init();
    adc_gpio_init(X_PIN); // X

    adc_init();
    adc_gpio_init(Y_PIN); //Y

    int zone_limit = 80;
    int mouse_speed = 10;
    
    while (1) {
        //X
        adc_select_input(1); // Select ADC input 1 (GPIO27)
        int x = adc_read();
        //printf("X: %d V\n", x);
        //Calcula a deadzone
        x = ((x-2047)/20);
        if (x <=zone_limit && x >= -1*(zone_limit)) {
            x = 0;
        }
        if (x > 0) {
            x = mouse_speed;
        }
        if (x < 0) {
            x = -mouse_speed;
        }
        struct mouse mouse_data_x = {0,(int)x};
        // printf("X: %d\n", x); // Debug
        if (x != 0)
            xQueueSend(xQueueMouse, &mouse_data_x, 1);
        
        //Y
        adc_select_input(2); // Select ADC input 2 (GPIO28)
        int y = adc_read();
        //printf("Y: %d V\n", y);
        //Calcula a deadzone
        y = ((y-2047)/20);
        if (y <=zone_limit && y >= -1*(zone_limit)) {
            y = 0;
        }
        if (y > 0) {
            y = mouse_speed;
        }
        if (y < 0) {
            y = -mouse_speed;
        }
        struct mouse mouse_data_y = {1,(int)y};
        // printf("Y: %d\n", y); // Debug

        if (y != 0)
            xQueueSend(xQueueMouse, &mouse_data_y, 1);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void hc05_task(void *p) {
    uart_init(hc05_UART_ID, hc05_BAUD_RATE);
    gpio_set_function(hc05_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(hc05_RX_PIN, GPIO_FUNC_UART);
    hc05_init("Carushow", "1234");
    gpio_init(hc05_PIN);
    gpio_set_dir(hc05_PIN, GPIO_IN);

    while(true){
        if (gpio_get(hc05_PIN) == 1) {
            xTaskCreate(mouse_task, "Mouse_Task", 4, NULL, 1, NULL);
            xTaskCreate(seletor_task, "Display", 4095, NULL, 1, NULL);
            xTaskCreate(botao_task, "Botao_Task", 4095, NULL, 1, NULL);
            break;
        }
    }

    // hc06_init("BTDeck", "bloons"); // Comentar para debugar localmente

    //if hc05_chack_connection();

    char letra;
    char msg[2];
    struct mouse mouse_data;
    while (true) {
        // uart_puts(hc05_UART_ID, "OLAAA ");
       // sprintf(msg,"%c ", letra);
        if (xQueueReceive(xQueueLetra, &letra, pdMS_TO_TICKS(100))) {
            uart_putc_raw(hc05_UART_ID, 0);
            uart_putc_raw(hc05_UART_ID, letra);
            uart_putc_raw(hc05_UART_ID, 0);
            uart_putc_raw(hc05_UART_ID, 1);
            uart_putc_raw(hc05_UART_ID, -1);
            //uart_puts(hc05_UART_ID, msg);
        //     vTaskDelay(pdMS_TO_TICKS(100));  
        }
        if (xQueueReceive(xQueueMouse, &mouse_data, pdMS_TO_TICKS(100))) {
            int val = mouse_data.val;
            int msb = val >> 8;
            int lsb = val & 0xFF ;
            uart_putc_raw(hc05_UART_ID, 1);
            uart_putc_raw(hc05_UART_ID, mouse_data.axis);
            uart_putc_raw(hc05_UART_ID, msb);
            uart_putc_raw(hc05_UART_ID, lsb);
            uart_putc_raw(hc05_UART_ID, -1);
           // mouse_write_package(mouse_data);
            // vTaskDelay(pdMS_TO_TICKS(100));
        }

    }
}

int main() {
    // stdio_init_all();
    // uart_init(uart0, 115200);
    // gpio_set_function(0, GPIO_FUNC_UART);
    // gpio_set_function(1, GPIO_FUNC_UART);
    //printf("Start bluetooth task\n");

    //Semaforos

    //Filas
    xQueueBTN = xQueueCreate(32, sizeof(uint16_t));
    xQueueMouse = xQueueCreate(4, sizeof(mouse_t));
    xQueueLetra = xQueueCreate(32, sizeof(char));
    xQueueClasse = xQueueCreate(32, sizeof(int));
    xQueueEnter = xQueueCreate(32, sizeof(int));


    //Tasks
    xTaskCreate(hc05_task, "UART_Task 1", 4096, NULL, 1, NULL);
    // xTaskCreate(mouse_task, "Mouse_Task", 4095, NULL, 1, NULL);
    // xTaskCreate(seletor_task, "Display", 4095, NULL, 1, NULL);
    // xTaskCreate(botao_task, "Botao_Task", 4095, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
