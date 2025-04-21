# 🚍 Sistema de Monitoramento Embarcado com Raspberry Pi Pico 🌟

Bem-vindo ao **Sistema de Monitoramento Embarcado**! Este projeto utiliza o **Raspberry Pi Pico** para controlar uma matriz de LEDs WS2812B, um display OLED SSD1306, LEDs RGB, um buzzer e botões, simulando um sistema de monitoramento (ex.: ônibus) com exibição de distância e tempo. Interaja via botões ou terminal serial e receba feedback visual e sonoro! 🎮💡

---

## 📖 Visão Geral

Este sistema embarcado simula um painel de controle para monitoramento de transporte, exibindo:
- **Dígitos/padrões** (0–4) em uma matriz de LEDs 5x5.
- **Distância percorrida** (0–100 km) e **tempo restante** (0–80 min) no display OLED.
- **Feedback visual** com LEDs RGB e **sonoro** com buzzer.

**Componentes**:
- **Matriz de LEDs WS2812B** (25 LEDs, pino 7) 🌈
- **Display OLED SSD1306** (I2C, pinos 14–15) 📺
- **LEDs RGB** (pinos 11–13) 💡
- **Buzzer** (pino 21) 🎵
- **Botões** (A: pino 5, B: pino 6, C: pino 22) 🖱️
- **Joystick (ADC)** (pino 26, EIXO_Y) 🎯

---

## ✨ Funcionalidades

- **Exibição de Dígitos**: Mostra padrões (0–4) na matriz de LEDs via comandos seriais ('0'–'4').
- **Monitoramento de Distância**: Exibe distância (0, 25, 50, 75, 100 km) no OLED com botão A ou comando '!'. LEDs RGB indicam estados.
- **Monitoramento de Tempo**: Exibe tempo (80, 60, 40, 20, 0 min) no OLED com botão B ou comando '#'. LEDs RGB atualizam.
- **Alarme**: Botão C aciona LED vermelho e buzzer (3350 Hz, 500 ms), exibindo "ALARME".
- **Interação Serial**: Comandos via terminal ('0'–'4', '!', '#') controlam exibições.
- **Debounce**: Filtra ruídos de botões com intervalo de 250 ms.

---

## 🛠️ Requisitos

- **Hardware**:
  - Raspberry Pi Pico
  - Matriz de LEDs WS2812B (5x5)
  - Display OLED SSD1306 (I2C)
  - LEDs RGB (vermelho, verde, azul)
  - Buzzer
  - 3 botões (pull-up)
  - Joystick (ou potenciômetro para ADC)
  - Fios, resistores e protoboard

- **Software**:
  - [Pico SDK](https://github.com/raspberrypi/pico-sdk) 📚
  - Compilador C/C++ (ex.: GCC para ARM)
  - Ferramenta de upload (ex.: `picotool` ou IDE como VS Code)
  - Terminal serial (ex.: PuTTY, Minicom)

---

## 🔌 Configuração

1. **Conexões de Hardware**:
   - Conecte a matriz de LEDs ao pino 7.
   - Ligue o display OLED aos pinos 14 (SDA) e 15 (SCL).
   - Conecte LEDs RGB aos pinos 11 (verde), 12 (azul), 13 (vermelho).
   - Buzzer ao pino 21.
   - Botões aos pinos 5 (A), 6 (B), 22 (C) com pull-up.
   - Joystick (EIXO_Y) ao pino 26 (ADC0).

2. **Ambiente de Software**:
   - Clone o repositório:
     ```bash
     git clone <URL_DO_REPOSITORIO>
     ```
   - Configure o Pico SDK no ambiente de desenvolvimento.
   - Copie os arquivos `ws2818b.pio` e `inc/ssd1306.h` para o projeto.

3. **Compilação e Upload**:
   - Compile o código com o Pico SDK:
     ```bash
     mkdir build && cd build
     cmake ..
     make
     ```
   - Faça upload do arquivo `.uf2` para o Pico via USB.

---

## 🚀 Como Usar

1. **Ligue o Sistema**:
   - Conecte o Pico a uma fonte de energia (USB ou externa).
   - O display OLED será inicializado vazio, e a matriz de LEDs será limpa.

2. **Interação com Botões**:
   - **Botão A (pino 5)**: Alterna exibição de distância no OLED.
   - **Botão B (pino 6)**: Alterna exibição de tempo no OLED.
   - **Botão C (pino 22)**: Ativa/desativa alarme (LED vermelho + buzzer).

3. **Comandos via Terminal Serial**:
   - Conecte-se ao Pico via terminal (ex.: 115200 baud rate).
   - Envie comandos:
     - `'0'–'4'`: Exibe dígito/padrão na matriz de LEDs.
     - `'!'`: Exibe distância no OLED.
     - `'#'`: Exibe tempo no OLED.

4. **Monitoramento**:
   - Ajuste o joystick (pino 26) para simular valores de ADC, afetando distância (0–100 km) e tempo (0–80 min).
   - LEDs RGB indicam estados (ex.: verde para distância máxima, azul para tempo elevado).
   - Mensagens no terminal fornecem feedback (ex.: "Distancia percorrida do ônibus: 50.00 km").

---

## 📋 Modos de Operação

| **Modo**            | **Entrada**             | **Saída**                                                                 |
|---------------------|-------------------------|---------------------------------------------------------------------------|
| **Dígitos**         | Comandos '0'–'4'        | Matriz de LEDs exibe padrão (0–4).                                        |
| **Distância**       | Botão A ou '!'          | OLED mostra distância (ex.: "50.00 km"); LEDs RGB atualizam (verde para 100 km). |
| **Tempo**           | Botão B ou '#'          | OLED mostra tempo (ex.: "40.00 min"); LEDs RGB atualizam (azul para >0). |
| **Alarme**          | Botão C                 | LED vermelho aceso, buzzer toca (500 ms), mensagem "ALARME" no terminal.  |
| **Ocioso**          | Comando '~' ou inativo  | Nenhuma ação; sistema aguarda entrada.                                    |

---

## ⚠️ Notas

- **Simulação**: Distância e tempo são simulados via ADC (joystick, pino 26). Para uso real, integre sensores.
- **Limitações**:
  - Apenas 5 dígitos (0–4) na matriz; EIXO_X (pino 27) não usado.
  - Comando '@' (botão A) definido, mas não processado.
- **Melhorias**:
  - Adicione sensores reais (ex.: GPS para distância).
  - Expanda padrões da matriz de LEDs.
  - Ajuste o intervalo de debounce (250 ms) para maior responsividade.

---

## 🎉 Contribuições

Sinta-se à vontade para contribuir! 🚀
- Reporte bugs ou sugira melhorias via **Issues**.
- Envie **Pull Requests** com novas funcionalidades ou correções.

---

## 📜 Licença

Este projeto é distribuído sob a licença **MIT**. Veja o arquivo `LICENSE` para detalhes.

---

**Desenvolvido com 💖 para entusiastas de eletrônica e IoT!**