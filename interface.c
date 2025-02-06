#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "neopixel.h"
#include "pico/bootrom.h"
#include "inc/ssd1306.h"


// Configuração da UART
#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0 
#define UART_RX_PIN 1 
#define BUFFER_SIZE 100 // Tamanho máximo da string

#define RED_PIN 13
#define GREEN_PIN 11
#define BLUE_PIN 12

#define BUTTON_A 5
#define BUTTON_B 6
#define BUTTON_JOYSTICK 22

static volatile uint32_t last_time_button_a = 0; // Armazena o tempo do último evento (em microssegundos)
static volatile uint32_t last_time_button_b = 0; // Armazena o tempo do último evento (em microssegundos)
static volatile uint32_t last_time_button_joystick = 0; // Armazena o tempo do último evento (em microssegundos)

static void gpio_button_a_handler(uint gpio, uint32_t events);
static void gpio_button_b_handler(uint gpio, uint32_t events);
static void gpio_button_joystick_handler(uint gpio, uint32_t events);
static void gpio_button_handler(uint gpio, uint32_t events);

static ssd1306_t ssd; // Inicializa a estrutura do display
void display_init();

void setup();

char c;
void atualiza_display();


int main()
{
    setup();

    atualiza_display();

    while (true) {
        c = fgetc(stdin);  // Lê um caractere do fluxo de entrada padrão
        if(c >= '0' && c <= '9'){
            exibirNumeroComFundo(c - '0',
                                0, 10, 0,
                                0, 0, 0);

        }
        if(c == '*'){
            npClear();
            npWrite();
        }

        // Exemplo de lógica adicional (como LEDs, botões, etc.)
        printf("%c\n", c);
        atualiza_display();
        sleep_ms(1000);  // Espera um pouco, mas o programa não fica bloqueado
    }
}

void setup(){

    // Inicializa a biblioteca padrão
    stdio_init_all();

    //Configura Matriz de Leds
    npInit(LED_PIN);

    //Configura Display
    display_init();

    // Configura LEDs
    gpio_init(GREEN_PIN);              
    gpio_set_dir(GREEN_PIN, GPIO_OUT); 
    gpio_put(GREEN_PIN, false );         // Inicialmente desligado

    gpio_init(BLUE_PIN);              
    gpio_set_dir(BLUE_PIN, GPIO_OUT); 
    gpio_put(BLUE_PIN, false);        // Inicialmente desligado

    // Configura botões
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    gpio_init(BUTTON_JOYSTICK);
    gpio_set_dir(BUTTON_JOYSTICK, GPIO_IN);
    gpio_pull_up(BUTTON_JOYSTICK);

    // Configura interrupções dos botões
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_button_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_button_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_JOYSTICK, GPIO_IRQ_EDGE_FALL, true, &gpio_button_handler);

    // Configura a UART
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART); // Configura o pino 0 para TX
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART); // Configura o pino 1 para RX

}

static void gpio_button_handler(uint gpio, uint32_t events) {
    switch(gpio) {
        case BUTTON_A:
            gpio_button_a_handler(gpio, events);
            break;
        case BUTTON_B:
            gpio_button_b_handler(gpio, events);
            break;
        case BUTTON_JOYSTICK:
            gpio_button_joystick_handler(gpio, events);
            break;
    }
}

static void gpio_button_a_handler(uint gpio, uint32_t events){
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if (current_time - last_time_button_a > 200000) {
        last_time_button_a = current_time;
        if(gpio_get(GREEN_PIN)) {
            gpio_put(GREEN_PIN, false); 
            printf("LED VERDE DESLIGADO!\n");
        } else {
            gpio_put(GREEN_PIN, true); 
            printf("LED VERDE LIGADO!\n");
        }
        atualiza_display();
    }
}

static void gpio_button_b_handler(uint gpio, uint32_t events){
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if (current_time - last_time_button_b > 200000) {
        last_time_button_b = current_time;
        if(gpio_get(BLUE_PIN)) {
            gpio_put(BLUE_PIN, false); 
            printf("LED AZUL DESLIGADO!\n");
            
        } else {
            gpio_put(BLUE_PIN, true); 
            printf("LED AZUL LIGADO!\n");
        }
        atualiza_display();
    }
}

static void gpio_button_joystick_handler(uint gpio, uint32_t events){
    printf("HABILITANDO O MODO GRAVAÇÃO");
    reset_usb_boot(0,0);
}

void display_init(){
  // I2C Initialisation. Using it at 400Khz.
  i2c_init(I2C_PORT, 400 * 1000);

  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
  gpio_pull_up(I2C_SDA); // Pull up the data line
  gpio_pull_up(I2C_SCL); // Pull up the clock line
  ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
  ssd1306_config(&ssd); // Configura o display
  ssd1306_send_data(&ssd); // Envia os dados para o display

  // Limpa o display. O display inicia com todos os pixels apagados.
  ssd1306_fill(&ssd, false);
  ssd1306_send_data(&ssd);
}

void atualiza_display(){

    char string[25] = "";
    string[0] = c;
    string[1] = '\0';
    ssd1306_fill(&ssd, false);
    ssd1306_draw_string(&ssd,
        gpio_get(BLUE_PIN) ? "AZUL LIGADO" : "AZUL DESLIGADO",
        5, 10);    
    ssd1306_draw_string(&ssd, 
        gpio_get(GREEN_PIN) ? "VERDE LIGADO" : "VERDE DESLIGADO",
        5, 30); // Desenha uma string
    ssd1306_draw_string(&ssd, string, 62, 50); // Desenha uma string      
    ssd1306_send_data(&ssd); // Atualiza o display
}