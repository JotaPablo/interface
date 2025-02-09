# Projeto de Comunicação Serial com RP2040 - BitDogLab

Projeto desenvolvido para a Unidade 4 - Capítulo 6, demonstrando o uso de interfaces seriais (UART, I2C) e controle de periféricos na placa BitDogLab com RP2040.

## Descrição
Implementação de um sistema interativo que combina:
- Comunicação serial via USB/UART
- Controle de display OLED via I2C
- Gerenciamento de LEDs endereçáveis WS2812
- Controle de LEDs RGB comuns
- Entrada de dados através de botões com tratamento de interrupções

## Ferramenta de Fontes
- Arquivo `gera_fontes.c` converte sprites do Piskel em código para a biblioteca `font.h`.
- Suporte a caracteres minúsculos e especiais.

## Componentes Utilizados
- Placa BitDogLab com RP2040
- Matriz de LEDs WS2812 5x5 (GPIO 7)
- LED RGB (GPIOs 11-13)
- Botões A (GPIO 5) e B (GPIO 6)
- Botão do JoyStick (GPIO 22)
- Display OLED SSD1306 via I2C (GPIO 14/15)

## Funcionalidades Principais

### Entrada Serial
- Leitura de caracteres via USB
- Exibição do caractere no display OLED
- Números (0-9) acionam padrões correspondentes na matriz de LEDs

### Controle por Botões
- **Botão A**: Alterna o LED Verde (ligado/desligado) e atualiza o display com a mensagem correspondente.
- **Botão B**: Alterna o LED Azul (ligado/desligado) e atualiza o display com a mensagem correspondente.
- **Botão do Joystick**: Ao ser pressionado, entra no modo **BOOTSEL** (modo de carregamento de firmware via USB).

### Controle de LEDs
- A matriz WS2812 5x5 exibe padrões numéricos quando um número (0-9) é digitado no monitor serial.
- Ao digitar o caractere `!`, a matriz de LEDs é apagada.
- LEDs RGB controlados por GPIOs (11-13) com estado persistente e feedback no display OLED.

### Display OLED
- Exibe em tempo real:
  - Estado dos LEDs RGB (verde e azul)
  - Último caractere recebido via entrada serial
### Interrupções e Debouncing
- Interrupções (IRQ): Todos os botões (A, B, Joystick) são monitorados por interrupções, garantindo uma resposta rápida e eficiente a cada pressão de botão.
- Debouncing de Botões: Foi implementada uma solução de debouncing via software para garantir que o pressionamento de botões seja reconhecido de forma precisa, sem falsos acionamentos causados por ruídos.

## ⚙️ Instalação e Uso

1. **Pré-requisitos**
   - Clonar o repositório:
     ```bash
     git clone https://github.com/JotaPablo/interface.git
     cd interrupcao
     ```
   - Instalar o **Visual Studio Code** com as seguintes extensões:
     - **Raspberry Pi Pico SDK**
     - **Compilador ARM GCC**

2. **Compilação**
   - Compile o projeto no terminal:
     ```bash
     mkdir build
     cd build
     cmake ..
     make
     ```
   - Ou utilize a extensão da Raspberry Pi Pico no VS Code.

3. **Execução**
   - **Na placa física:** 
     - Conecte a placa ao computador em modo **BOOTSEL**.
     - Copie o arquivo `.uf2` gerado na pasta `build` para o dispositivo identificado como `RPI-RP2`, ou envie através da extensão da Raspberry Pi Pico no VS Code.
  
