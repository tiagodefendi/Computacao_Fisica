# Atividade 01 - Revisão de Conceitos. Livro AVR e Arduino–Técnicas de Projeto

> Capítulos: 1-Introdução e 2-O ATmega328 e introdução ao ESP32
>
> *Obs.: Deve ser entregue arquivo contendo as perguntas e respectivas respostas.*

1. Quanto à organização do barramento, existem duas arquiteturas predominantes para as CPUs dos microprocessadores, a arquitetura Von-Neumann e a arquitetura Harvard. Explique as características de cada uma delas. Utilize o diagrama da Figura 2.1, que mostra a arquitetura Harvard empregada pelo ATmega328, para ressaltar as características importantes desta arquitetura que permitem uma relação praticamente 1:1 entre MIPS e MHz.

# Parte prática

Título: Entendendo os conceitos de entrada e saída–diferentes características das portas

Objetivos:Familiarização com o Tinkercad e o Wokwi para a simulação de circuitos simples. Compreender como funciona a entrada e saída no Atmega328.

Nesta prática utilizaremos o Tinkercad e o Wokwi para simular um circuito simples usando o microcontrolador Atmega328p, utilizado nas placas Arduino UNO e ESP32.

# Procedimentos:

1. Acesse sua conta no Tinkercad (tinkercad.com). Em seguida, vá para a abacircuits(https://www.tinkercad.com/circuits).

2. A partir do código piscar leds exemplo, ou a partir da prática 0, adicione 2 osciloscópios como na figura. Note que o osciloscópio plota a diferença de voltagem ao longo do tempo e com isto, consegue exibir uma forma de onda.

3. Altere o intervalo de tempo dos delays de forma a obter o led piscando a 10Hz, ou seja, 10 vezes por segundo. Coloque a porta que está acendendo o led como entrada e depois como saída. Cole a imagem dos osciloscópios nos dois casos. O que acontece com a voltagem? Por quê?

Experimente outras configurações, troque o valor do resistor, troque o led por um resistor, etc. Veja o que acontece com a saída.

4. Refaça o projeto da atividade 0 no Wokwi (https://wokwi.com/) com uma placa ESP32. Desta vez, utilize os pinos D14 e D15 para o LED1 e LED2, respectivamente. Reescreva o código de maneira a não utilizar nem pinMode e nem digitalWrite. Você vai utilizar os macros REG_SET_BIT e os registradores GPIO_ENABLE_REG, GPIO_OUT_W1TS_REG e GPIO_OUT_W1TC_REG. Este site pode ser útil (https://portal.vidadesilicio.com.br/manipulando-os-registradores-esp32/(mirror:https://moodle.utfpr.edu.br/mod/resource/view.php?id=1517776)) e este é o link para as macros (https://github.com/espressif/esp-idf/blob/master/components/soc/esp32/include/soc/soc.h).
