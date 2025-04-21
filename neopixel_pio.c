/* 
 * Programa para controle de um sistema embarcado com Raspberry Pi Pico.
 * Controla uma matriz de LEDs WS2812B, display OLED SSD1306, LEDs RGB, buzzer e botões.
 * Utiliza ADC para simular leituras de distância e tempo, exibidas no display e matriz de LEDs.
 * Suporta entrada de comandos via terminal e interrupções de botões.
 */

// Inclusão de bibliotecas padrão e específicas do Raspberry Pi Pico
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"       // Funções padrão do Pico
#include "hardware/pio.h"      // Interface para Programmed I/O
#include "hardware/clocks.h"   // Controle de clocks
#include "hardware/gpio.h"     // Controle de pinos GPIO
#include "hardware/i2c.h"      // Comunicação I2C
#include "hardware/adc.h"      // Conversor Analógico-Digital
#include "hardware/pwm.h"      // Modulação por largura de pulso
#include "ws2818b.pio.h"       // Programa PIO para LEDs WS2812B
#include "inc/ssd1306.h"       // Biblioteca para display OLED SSD1306

// Definições de pinos usados no hardware
#define LED_COUNT 25           // Número de LEDs na matriz
#define LED_PIN 7              // Pino para a matriz de LEDs WS2812B
#define WS2812_PIN 7           // Mesmo pino que LED_PIN (mantido para compatibilidade)
#define EIXO_Y 26              // Pino ADC0 para eixo Y do joystick
#define EIXO_X 27              // Pino ADC1 para eixo X do joystick
#define BLUE_LED_PIN 12        // Pino do LED azul
#define RED_LED_PIN 13         // Pino do LED vermelho
#define GREEN_LED_PIN 11       // Pino do LED verde
#define BUZZER_PIN 21          // Pino do buzzer
#define BOTAO_A_PIN 5          // Pino do botão A
#define BOTAO_B_PIN 6          // Pino do botão B
#define BOTAO_C_PIN 22         // Pino do botão C (joystick)
#define I2C_SDA 14             // Pino SDA para comunicação I2C
#define I2C_SCL 15             // Pino SCL para comunicação I2C

// Definição do porto I2C usado
#define I2C_PORT i2c1          // Porta I2C1 para comunicação com o display OLED

// Estrutura para representar um pixel RGB na matriz de LEDs
struct pixel_t {
    uint8_t G, R, B;           // Componentes verde, vermelho e azul
};
typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t;       // Tipo para LEDs NeoPixel

// Variáveis globais
npLED_t leds[LED_COUNT];       // Array para armazenar estado dos LEDs
PIO np_pio;                    // Instância do PIO para controle da matriz de LEDs
uint sm;                       // Máquina de estado do PIO
volatile int current_digit = 0; // Dígito atual exibido na matriz de LEDs
volatile char c = '~';         // Último comando recebido (inicializado como '~')
volatile bool new_data = false;// Flag para indicar novo comando recebido
bool controle1 = false;        // Estado do botão A
bool controle2 = false;        // Estado do botão B
bool controle3 = false;        // Estado do botão C
uint distancia_global = 0;     // Distância calculada globalmente
uint tempo_global = 0;         // Tempo calculado globalmente
volatile bool botao_pressionado = false; // Flag para indicar botão pressionado
volatile uint8_t botao_gpio = 0;        // Pino do botão que gerou interrupção
absolute_time_t last_interrupt_time = 0;// Timestamp da última interrupção

// Matrizes para exibição de dígitos na matriz de LEDs (5x5 pixels, RGB)
// Cada dígito/situação é representado por uma matriz de cores
const uint8_t digits[11][5][5][3] = {
    // Situação 1 (Rodoviária): Linha superior roxa, um pixel azul na última linha
    {
        {{100, 0, 50}, {100, 0, 50}, {100, 0, 50}, {100, 0, 50}, {100, 0, 50}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 255}, {0, 0, 0}, {0, 0, 0}}
    },
    // Dígito 1: Linha superior roxa, pixel azul na penúltima linha
    {
        {{100, 0, 50}, {100, 0, 50}, {100, 0, 50}, {100, 0, 50}, {100, 0, 50}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 255}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
    },
    // Dígito 2: Linha superior roxa, pixel azul na terceira linha
    {
        {{100, 0, 50}, {100, 0, 50}, {100, 0, 50}, {100, 0, 50}, {100, 0, 50}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 255}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
    },
    // Dígito 3: Linha superior roxa, pixel azul na segunda linha
    {
        {{100, 0, 50}, {100, 0, 50}, {100, 0, 50}, {100, 0, 50}, {100, 0, 50}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 255}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
    },
    // Dígito 4: Linha superior com um pixel azul, outros roxos
    {
        {{100, 0, 50}, {100, 0, 50}, {100, 0, 255}, {100, 0, 50}, {100, 0, 50}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
    }
};

// Protótipos de funções (declarações para uso posterior)
void init_leds_and_buzzer();
void pwm_init_buzzer(uint pin);
void play_buzzer(uint pin, uint frequency, uint duration_ms);
void npSetLED(uint index, uint8_t r, uint8_t g, uint8_t b);
void npClear();
void npInit(uint pin);
void npWrite();
void npDisplayDigit(int digit);
int getIndex(int x, int y);
float CalcularDistancia();
float CalcularTempo();
void process_command(int digit, char *line1, uint8_t *ssd, struct render_area *frame_area);
void process_command_distancia(char c, char *line1, uint8_t *ssd, struct render_area *frame_area, float distancia);
void process_command_tempo(char c, char *line1, uint8_t *ssd, struct render_area *frame_area, float tempo);
void gpio_callback(uint gpio, uint32_t events);
void tratar_botoes_e_display();

// Inicializa os LEDs RGB e o buzzer como saídas
void init_leds_and_buzzer() {
    gpio_init(RED_LED_PIN);    // Inicializa pino do LED vermelho
    gpio_set_dir(RED_LED_PIN, GPIO_OUT); // Configura como saída
    gpio_put(RED_LED_PIN, false); // Desliga inicialmente

    gpio_init(GREEN_LED_PIN);  // Inicializa pino do LED verde
    gpio_set_dir(GREEN_LED_PIN, GPIO_OUT);
    gpio_put(GREEN_LED_PIN, false);

    gpio_init(BLUE_LED_PIN);   // Inicializa pino do LED azul
    gpio_set_dir(BLUE_LED_PIN, GPIO_OUT);
    gpio_put(BLUE_LED_PIN, false);

    gpio_init(BUZZER_PIN);     // Inicializa pino do buzzer
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    gpio_put(BUZZER_PIN, false);
}

// Configura o PWM para o buzzer
void pwm_init_buzzer(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM); // Define pino como PWM
    uint slice_num = pwm_gpio_to_slice_num(pin); // Obtém slice PWM
    pwm_config config = pwm_get_default_config(); // Configuração padrão
    pwm_init(slice_num, &config, true); // Inicializa PWM
    pwm_set_gpio_level(pin, 0); // Define nível inicial como 0
}

// Toca um som no buzzer com frequência e duração especificadas
void play_buzzer(uint pin, uint frequency, uint duration_ms) {
    gpio_set_function(pin, GPIO_FUNC_PWM); // Configura pino como PWM
    uint slice_num = pwm_gpio_to_slice_num(pin); // Obtém slice PWM
    pwm_config config = pwm_get_default_config(); // Configuração padrão
    // Ajusta divisor de clock para atingir a frequência desejada
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (frequency * 4096));
    pwm_init(slice_num, &config, true); // Inicializa PWM
    pwm_set_gpio_level(pin, 2048); // Define duty cycle de ~50%
    sleep_ms(duration_ms); // Aguarda a duração especificada
    pwm_set_gpio_level(pin, 0); // Desliga o som
    pwm_set_enabled(slice_num, false); // Desativa PWM
}

// Define as cores de um LED na matriz
void npSetLED(uint index, uint8_t r, uint8_t g, uint8_t b) {
    leds[index].R = r; // Define componente vermelho
    leds[index].G = g; // Define componente verde
    leds[index].B = b; // Define componente azul
}

// Limpa a matriz de LEDs, exibindo o dígito 10 (padrão para limpar)
void npClear() {
    current_digit = 10;
    npDisplayDigit(current_digit);
}

// Inicializa a matriz de LEDs WS2812B usando PIO
void npInit(uint pin) {
    uint offset = pio_add_program(pio0, &ws2818b_program); // Carrega programa PIO
    np_pio = pio0; // Usa PIO0
    sm = pio_claim_unused_sm(np_pio, true); // Reserva máquina de estado
    // Inicializa programa PIO com pino e frequência
    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);
    npClear(); // Limpa a matriz
}

// Escreve os dados dos LEDs na matriz
void npWrite() {
    for (uint i = 0; i < LED_COUNT; i++) {
        // Envia componentes G, R, B para o PIO em sequência
        pio_sm_put_blocking(np_pio, sm, leds[i].G);
        pio_sm_put_blocking(np_pio, sm, leds[i].R);
        pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
    sleep_us(100); // Pequeno atraso para estabilizar
}

// Calcula o índice de um LED na matriz com base em coordenadas (x, y)
int getIndex(int x, int y) {
    if (y % 2 == 0) {
        return 24 - (y * 5 + x); // Linhas pares: ordem direta
    } else {
        return 24 - (y * 5 + (4 - x)); // Linhas ímpares: ordem invertida
    }
}

// Calcula a distância com base na leitura do ADC (simulação)
float CalcularDistancia() {
    uint distancia_12bits = adc_read(); // Lê valor ADC (0 a 4096)
    if (distancia_12bits > 0 && distancia_12bits <= 512) {
        distancia_global = 0; // Distância 0 km
    } else if (distancia_12bits > 512 && distancia_12bits <= 1024) {
        distancia_global = 25; // Distância 25 km
    } else if (distancia_12bits > 1024 && distancia_12bits <= 2048) {
        distancia_global = 50; // Distância 50 km
    } else if (distancia_12bits > 2048 && distancia_12bits <= 3000) {
        distancia_global = 75; // Distância 75 km
    } else if (distancia_12bits > 3000 && distancia_12bits <= 4096) {
        distancia_global = 100; // Distância 100 km
        gpio_put(BLUE_LED_PIN, 0); // Desliga LED azul
        gpio_put(GREEN_LED_PIN, 1); // Acende LED verde
    }
    return distancia_global; // Retorna distância calculada
}

// Calcula o tempo com base na leitura do ADC (simulação)
float CalcularTempo() {
    uint tempo_12bits = adc_read(); // Lê valor ADC (0 a 4096)
    uint tempo;
    if (tempo_12bits > 0 && tempo_12bits <= 512) {
        tempo = 80; // Tempo 80 minutos
        gpio_put(RED_LED_PIN, 0); // Desliga LED vermelho
        gpio_put(GREEN_LED_PIN, 0); // Desliga LED verde
        gpio_put(BLUE_LED_PIN, 1); // Acende LED azul
    } else if (tempo_12bits > 512 && tempo_12bits <= 1024) {
        tempo = 60; // Tempo 60 minutos
        gpio_put(RED_LED_PIN, 0);
        gpio_put(GREEN_LED_PIN, 0);
        gpio_put(BLUE_LED_PIN, 1);
    } else if (tempo_12bits > 1024 && tempo_12bits <= 2048) {
        tempo = 40; // Tempo 40 minutos
        gpio_put(RED_LED_PIN, 0);
        gpio_put(GREEN_LED_PIN, 0);
        gpio_put(BLUE_LED_PIN, 1);
    } else if (tempo_12bits > 2048 && tempo_12bits <= 3000) {
        tempo = 20; // Tempo 20 minutos
        gpio_put(RED_LED_PIN, 0);
        gpio_put(BLUE_LED_PIN, 1);
        gpio_put(GREEN_LED_PIN, 0);
    } else if (tempo_12bits > 3000 && tempo_12bits <= 4096) {
        tempo = 0; // Tempo 0 minutos
        gpio_put(RED_LED_PIN, 0);
        gpio_put(BLUE_LED_PIN, 0);
        gpio_put(GREEN_LED_PIN, 1);
    }
    return tempo; // Retorna tempo calculado
}

// Processa comando para exibir um dígito na matriz de LEDs
void process_command(int digit, char *line1, uint8_t *ssd, struct render_area *frame_area) {
    current_digit = digit; // Define dígito atual
    npDisplayDigit(current_digit); // Exibe dígito na matriz
}

// Processa comando para exibir distância no display OLED
void process_command_distancia(char c, char *line1, uint8_t *ssd, struct render_area *frame_area, float distancia) {
    if (strchr("!@#$", c) != NULL) {
        sleep_ms(5); // Pequeno atraso para comandos especiais
    } else {
        printf("O comando foi %c\n", c); // Exibe comando recebido
    }

    memset(ssd, 0, ssd1306_buffer_length); // Limpa buffer do display
    render_on_display(ssd, frame_area); // Renderiza buffer vazio

    char distancia_str[32];
    snprintf(distancia_str, sizeof(distancia_str), "%.2f km", distancia); // Formata distância
    printf("Distancia percorrida do ônibus: %.2f km\n", distancia); // Exibe no terminal
    ssd1306_draw_string(ssd, 5, 0, line1); // Exibe texto da primeira linha
    ssd1306_draw_string(ssd, 5, 8, distancia_str); // Exibe distância
    render_on_display(ssd, frame_area); // Renderiza no display
}

// Processa comando para exibir tempo no display OLED
void process_command_tempo(char c, char *line1, uint8_t *ssd, struct render_area *frame_area, float tempo) {
    if (strchr("!@#$", c) != NULL) {
        sleep_ms(5); // Pequeno atraso para comandos especiais
    } else {
        printf("O comando foi %c\n", c); // Exibe comando recebido
    }

    memset(ssd, 0, ssd1306_buffer_length); // Limpa buffer do display
    render_on_display(ssd, frame_area); // Renderiza buffer vazio

    char tempo_str[32];
    snprintf(tempo_str, sizeof(tempo_str), "%.2f minutos", tempo); // Formata tempo
    printf("Tempo para o ônibus chegar: %.2f minutos\n", tempo); // Exibe no terminal
    ssd1306_draw_string(ssd, 5, 0, line1); // Exibe texto da primeira linha
    ssd1306_draw_string(ssd, 5, 8, tempo_str); // Exibe tempo
    render_on_display(ssd, frame_area); // Renderiza no display
}

// Callback de interrupção para botões
void gpio_callback(uint gpio, uint32_t events) {
    absolute_time_t now = get_absolute_time(); // Obtém tempo atual
    int64_t diff = absolute_time_diff_us(last_interrupt_time, now); // Calcula diferença
    if (diff < 2500) return; // Debounce de 250ms
    last_interrupt_time = now; // Atualiza tempo da última interrupção

    botao_gpio = gpio; // Armazena pino que gerou interrupção
    botao_pressionado = true; // Sinaliza botão pressionado
}

// Trata eventos de botões e atualiza o display
void tratar_botoes_e_display() {
    if (!botao_pressionado) return; // Sai se nenhum botão foi pressionado

    botao_pressionado = false; // Reseta flag

    // Define área de renderização do display
    struct render_area frame_area = {
        .start_column = 0,
        .end_column = ssd1306_width - 1,
        .start_page = 0,
        .end_page = ssd1306_n_pages - 1
    };

    uint distancia = CalcularDistancia(); // Calcula distância
    uint tempo = CalcularTempo(); // Calcula tempo

    // Botão A: Alterna exibição de distância
    if (botao_gpio == BOTAO_A_PIN) {
        controle1 = !controle1; // Alterna estado
        if (controle1) {
            c = '!'; // Comando para exibir distância
            new_data = true; // Sinaliza novo comando
        } else {
            c = '@'; // Comando alternativo (não usado)
        }
    // Botão B: Alterna exibição de tempo
    } else if (botao_gpio == BOTAO_B_PIN) {
        controle2 = !controle2;
        if (controle2) {
            new_data = true;
            c = '#'; // Comando para exibir tempo
        }
    // Botão C: Ativa alarme
    } else if (botao_gpio == BOTAO_C_PIN) {
        controle3 = !controle3;
        if (controle3) {
            new_data = true;
            printf("ALARME\n"); // Exibe mensagem de alarme
            gpio_put(BLUE_LED_PIN, 0); // Desliga LED azul
            gpio_put(GREEN_LED_PIN, 0); // Desliga LED verde
            gpio_put(RED_LED_PIN, 1); // Acende LED vermelho
            play_buzzer(BUZZER_PIN, 3350, 500); // Toca buzzer por 500ms
        }
    }
}

// Exibe um dígito na matriz de LEDs
void npDisplayDigit(int digit) {
    for (int coluna = 0; coluna < 5; coluna++) {
        for (int linha = 0; linha < 5; linha++) {
            int posicao = getIndex(linha, coluna); // Calcula índice do LED
            // Define cores do LED com base na matriz de dígitos
            npSetLED(
                posicao,
                digits[digit][coluna][linha][0], // R
                digits[digit][coluna][linha][1], // G
                digits[digit][coluna][linha][2]  // B
            );
        }
    }
    npWrite(); // Escreve na matriz
}

// Função principal do programa
int main() {
    stdio_init_all(); // Inicializa comunicação serial
    sleep_ms(1000); // Aguarda 1s para estabilizar

    // Inicializa ADC para leitura do joystick
    adc_init();
    adc_gpio_init(EIXO_Y); // Configura pino ADC
    adc_select_input(0); // Seleciona canal ADC0

    // Inicializa LEDs e buzzer
    init_leds_and_buzzer();

    // Configura botões como entradas com pull-up
    gpio_init(BOTAO_A_PIN);
    gpio_set_dir(BOTAO_A_PIN, GPIO_IN);
    gpio_pull_up(BOTAO_A_PIN);

    gpio_init(BOTAO_B_PIN);
    gpio_set_dir(BOTAO_B_PIN, GPIO_IN);
    gpio_pull_up(BOTAO_B_PIN);

    gpio_init(BOTAO_C_PIN);
    gpio_set_dir(BOTAO_C_PIN, GPIO_IN);
    gpio_pull_up(BOTAO_C_PIN);

    // Inicializa matriz de LEDs
    npInit(LED_PIN);

    // Inicializa I2C para comunicação com o display
    i2c_init(I2C_PORT, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Configura pinos I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicializa display OLED
    ssd1306_init();

    // Configura área de renderização do display
    uint8_t ssd[ssd1306_buffer_length]; // Buffer do display
    struct render_area frame_area = {
        .start_column = 0,
        .end_column = ssd1306_width - 1,
        .start_page = 0,
        .end_page = ssd1306_n_pages - 1
    };
    calculate_render_area_buffer_length(&frame_area); // Calcula tamanho do buffer
    memset(ssd, 0, ssd1306_buffer_length); // Limpa buffer
    render_on_display(ssd, &frame_area); // Renderiza display vazio

    // Configura interrupções para os botões
    gpio_set_irq_enabled(BOTAO_A_PIN, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BOTAO_B_PIN, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BOTAO_C_PIN, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_callback(gpio_callback); // Define callback de interrupção
    irq_set_enabled(IO_IRQ_BANK0, true); // Ativa interrupções GPIO

    // Loop principal
    while (true) {
        sleep_ms(50); // Atraso para estabilizar o sistema
        tratar_botoes_e_display(); // Processa eventos de botões
        uint tempo = CalcularTempo(); // Calcula tempo (usado para LEDs)

        // Verifica entrada de comandos via terminal
        int input = getchar_timeout_us(0); // Lê caractere sem bloqueio
        if (input != PICO_ERROR_TIMEOUT || new_data) {
            if (!new_data) {
                c = (char)input; // Converte entrada para char
            }
            // Processa comandos recebidos
            switch (c) {
                case '0': process_command(0, "numero", ssd, &frame_area); break; // Exibe dígito 0
                case '1': process_command(1, "numero", ssd, &frame_area); break; // Exibe dígito 1
                case '2': process_command(2, "numero", ssd, &frame_area); break; // Exibe dígito 2
                case '3': process_command(3, "numero", ssd, &frame_area); break; // Exibe dígito 3
                case '4': process_command(4, "numero", ssd, &frame_area); break; // Exibe dígito 4
                case '!': process_command_distancia(c, "Distancia", ssd, &frame_area, CalcularDistancia()); break; // Exibe distância
                case '#': process_command_tempo(c, "Tempo restante", ssd, &frame_area, CalcularTempo()); break; // Exibe tempo
                case '~': break; // Comando nulo (nenhuma ação)
            }
            new_data = false; // Reseta flag de novo comando
        }
    }
}