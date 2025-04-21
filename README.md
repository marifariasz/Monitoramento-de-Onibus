# 📡 Comunicação Serial com RP2040 – Projeto com BitDogLab  

Este projeto explora as interfaces de comunicação serial no microcontrolador **RP2040**, utilizando a placa **BitDogLab**. Implementamos a comunicação via **UART e I2C**, manipulação de LEDs (comuns e endereçáveis WS2812) e interação com botões utilizando **interrupções** e **debouncing**.  

## 🛠️ Funcionalidades  

✅ **Modificação da Biblioteca `font.h`**  
- Adição de caracteres minúsculos personalizados.  

✅ **Entrada de Caracteres via UART**  
- Recebe caracteres via **Serial Monitor (VS Code)** e os exibe no **display OLED SSD1306**.  
- Se um número (0-9) for digitado, um **símbolo correspondente** será exibido na matriz de LEDs **5x5 WS2812**.  

✅ **Interação com Botões**  
- **Botão A (GPIO 5):** Alterna o LED Verde do **LED RGB** e exibe mensagens no **display** e **Serial Monitor**.  
- **Botão B (GPIO 6):** Alterna o LED Azul do **LED RGB** e exibe mensagens no **display** e **Serial Monitor**.  

## 🔧 Componentes Utilizados  

| Componente | Conexão |  
|------------|---------|  
| Matriz WS2812 (5x5) | GPIO 7 |  
| LED RGB | GPIOs 11, 12, 13 |  
| Botão A | GPIO 5 |  
| Botão B | GPIO 6 |  
| Display SSD1306 (I2C) | GPIOs 14, 15 |  

## 📌 Requisitos Técnicos  

⚡ **Interrupções:** Todos os eventos dos botões são tratados via **IRQ**.  
🛑 **Debouncing:** Implementação por software para evitar leituras erradas dos botões.  
💡 **Controle de LEDs:** Uso de LEDs comuns e WS2812 com diferentes tipos de controle.  
📟 **Uso do Display OLED:** Exibição de caracteres maiúsculos, minúsculos e símbolos.  
🔗 **Envio de Informações via UART:** Comunicação serial entre o microcontrolador e o PC.  
📌 **Código Estruturado:** Código modular e bem comentado para fácil compreensão.  

## 🚀 Como Rodar o Projeto  

1️⃣ **Clone este repositório:**  
```bash  
git clone https://github.com/seu-usuario/seu-repositorio.git  
```
2️⃣ **Configure seu ambiente de desenvolvimento (VS Code + extensão do RP2040).**

3️⃣ **Compile e carregue o código na placa BitDogLab.**

4️⃣ **Utilize o Serial Monitor para testar a comunicação UART.**

5️⃣ **Interaja com os botões e veja as respostas no display e LEDs.**

## 🎥 Demonstração
📌 Vídeo de apresentação do projeto: [Clique aqui](https://youtu.be/OJgjAdRrYYs)

## 👩‍💻 Desenvolvedora
Projeto desenvolvido por Mariana Farias da Silva.
