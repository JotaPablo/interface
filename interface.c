#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "neopixel.h"
#include "pico/bootrom.h"
#include "inc/ssd1306.h"

// Definição dos pinos dos LEDs RGB
#define RED_PIN 13
#define GREEN_PIN 11
#define BLUE_PIN 12

// Definição dos pinos dos botões
#define BUTTON_A 5
#define BUTTON_B 6
#define BUTTON_JOYSTICK 22

// Variáveis para debounce dos botões (armazenam tempo do último acionamento)
static volatile uint32_t last_time_button_a = 0;
static volatile uint32_t last_time_button_b = 0;
static volatile uint32_t last_time_button_joystick = 0;

// Protótipos das funções de tratamento de interrupção
static void gpio_button_a_handler(uint gpio, uint32_t events);
static void gpio_button_b_handler(uint gpio, uint32_t events);
static void gpio_button_joystick_handler(uint gpio, uint32_t events);
static void gpio_button_handler(uint gpio, uint32_t events);

// Estrutura e funções do display OLED
static ssd1306_t ssd; // Estrutura de controle do display
void display_init();  // Inicialização do hardware

// Variáveis globais
void setup(); // Função de configuração inicial
int c;        // Armazena o caractere recebido via USB
void atualiza_display(); // Atualiza o conteúdo do OLED
bool ATUALIZA = false;   // Flag para controle de atualização do display

int main() {
    setup();
    atualiza_display();

    while (true) {
        // Verifica conexão USB e dados recebidos
        if (stdio_usb_connected()) {
            char aux = c;
            // Obtém caractere com timeout de 100ms
            c = getchar_timeout_us(100000);
            
            // Mantém último valor válido em caso de timeout
            if(c == PICO_ERROR_TIMEOUT) {
                c = aux;
            } else {
                // Processa caracteres numéricos
                if(c >= '0' && c <= '9') {
                    exibirNumeroComFundo(c - '0', 0, 10, 0, 0, 0, 0);
                }
                
                // Comando para limpar NeoPixels
                if(c == '!') {
                    npClear();
                    npWrite(); 
                }

                printf("%c\n", c); // Ecoa o caractere via USB
                ATUALIZA = true;   // Solicita atualização do display
            }
        }
        
        // Atualiza display se necessário
        if(ATUALIZA) {
            atualiza_display();
            ATUALIZA = false;
        }
    }
}

void setup() {
    // Inicialização de sistemas básicos
    stdio_init_all(); // USB, stdio
    
    // Configuração de hardware
    npInit(LED_PIN);  // Iniacializa NeoPixels
    
    display_init();    // Configura display OLED

    // Configura GPIOs dos LEDs
    gpio_init(GREEN_PIN);              
    gpio_set_dir(GREEN_PIN, GPIO_OUT); 
    gpio_put(GREEN_PIN, false); // Estado inicial desligado

    gpio_init(BLUE_PIN);              
    gpio_set_dir(BLUE_PIN, GPIO_OUT); 
    gpio_put(BLUE_PIN, false); // Estado inicial desligado

    // Configuração dos botões com pull-up
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    gpio_init(BUTTON_JOYSTICK);
    gpio_set_dir(BUTTON_JOYSTICK, GPIO_IN);
    gpio_pull_up(BUTTON_JOYSTICK);

    // Configura interrupções para borda de descida (botão pressionado)
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_button_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_button_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_JOYSTICK, GPIO_IRQ_EDGE_FALL, true, &gpio_button_handler);

}

// Tratador central de interrupções de botões
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

// Tratador do botão A (controle LED verde)
static void gpio_button_a_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    
    // Debounce: verifica se passaram pelo menos 200ms desde o último acionamento
    if (current_time - last_time_button_a > 200000) {
        last_time_button_a = current_time;
        
        // Alterna estado do LED
        bool estado = !gpio_get(GREEN_PIN);
        gpio_put(GREEN_PIN, estado); 
        printf("LED VERDE %s!\n", estado ? "LIGADO" : "DESLIGADO");
        
        ATUALIZA = true; // Solicita atualização do display
    }
}

// Tratador do botão B (controle LED azul)
static void gpio_button_b_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    
    // Debounce: verifica se passaram pelo menos 200ms
    if (current_time - last_time_button_b > 200000) {
        last_time_button_b = current_time;
        
        // Alterna estado do LED
        bool estado = !gpio_get(BLUE_PIN);
        gpio_put(BLUE_PIN, estado); 
        printf("LED AZUL %s!\n", estado ? "LIGADO" : "DESLIGADO");
        
        ATUALIZA = true; // Solicita atualização do display
    }
}

// Tratador do botão do joystick (entra em modo bootloader USB)
static void gpio_button_joystick_handler(uint gpio, uint32_t events) {
    printf("HABILITANDO O MODO GRAVAÇÃO");
    reset_usb_boot(0,0); // Reinicia no modo DFU
}

// Inicialização do display OLED
void display_init() {
    // Configuração I2C a 400kHz
    i2c_init(I2C_PORT, 400 * 1000);
    
    // Configura pinos I2C
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA); // Ativa pull-ups internos
    gpio_pull_up(I2C_SCL);

    // Inicialização do controlador SSD1306
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);

    // Limpa a tela inicialmente
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
}

// Atualiza o conteúdo do display OLED
void atualiza_display() {
    char string[15] = "Caractere: "; // Buffer para texto (cuidado com overflow!)
    string[11] = c; // Insere o caractere atual
    string[12] = '\0'; // Terminador de string
    
    // Limpa o display e escreve novos conteúdos
    ssd1306_fill(&ssd, false);
    
    // Exibe status dos LEDs
    ssd1306_draw_string(&ssd, 
        gpio_get(GREEN_PIN) ? "VERDE LIGADO" : "VERDE DESLIGADO",
        5, 10);
    ssd1306_draw_string(&ssd,
        gpio_get(BLUE_PIN) ? "AZUL LIGADO" : "AZUL DESLIGADO",
        5, 30);    

    
    // Exibe caractere recebido
    ssd1306_draw_string(&ssd, string, 5, 50);     
    ssd1306_send_data(&ssd); // Envia buffer para o display
}
