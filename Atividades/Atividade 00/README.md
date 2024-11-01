Atividade_00 – Conceitos básicos. ATmega328 e introdução ao ESP32

Obs.: Deve ser entregue arquivo contendo as perguntas e respectivas respostas.

1. Baixe os datasheets (folha de dados) do Atmel ATMega328p, do PIC PIC16F87XA (PIC16F877A), do Texas MSP430F249 e do STMicroelectronics STM8TL53F4.

Compare os quatro em termos de: memória (flash, eeprom, pilha, organização), conjunto de instruções, interrupções, portas de entrada e saída (quantidade, características?), periféricos (timers, ADC, protocolos de comunicação nativos).

Parte prática

Título: Conhecendo o Tinkercad e o Wokwi para simulação de circuitos

Objetivos: Familiarização com o Tinkercad e o Wokwi para a simulação de circuitos simples.

Nesta prática utilizaremos o Tinkercad e o Wokwi para simular um circuito simples usando o microcontrolador Atmega328p, utilizado nas placas Arduino UNO.

Procedimentos:

1. Crie uma conta no Tinkercad, caso não possua (tinkercad.com).

2. Em seguida, vá para a aba circuits (https://www.tinkercad.com/circuits).

3. Você deve fazer um circuito capaz de piscar um led. Note que este projeto já está disponível (na aba Starters → Arduino). Modifique para que o LED (LED1) seja ligado no pino 10 (PB2).

4. Modifique o projeto de forma a provocar flashs intermitentes. O led deve ficar apagado por 500ms e aceso por apenas 50ms.

5. Adicione um segundo led (LED2) que acende na sequência do primeiro (Deve ser ligado no pino 5, PD5). Assim, a sequência de ativação seria: LED1 (50ms), LED1 apaga e LED2 acende(50ms), 450ms (todos apagados), LED1 (50ms), …

6. Use um led amarelo para o LED1 e verde para o LED2.

7. Agora reescreva o código de maneira a não utilizar nem pinMode e nem digitalWrite.

8. Agora refaça todo o projeto no Wokwi. Crie uma conta no Wokwi caso não possua (https://wokwi.com/).

9. Agora utilize uma placa ESP32. Desta vez, utilize os pinos D27 e D23 para o LED1 e LED2, respectivamente.

10. Cole os códigos-fonte do microcontrolador ao final deste arquivo e inclua as imagens de seu design para os itens (5)6, 7, 8, 9;
