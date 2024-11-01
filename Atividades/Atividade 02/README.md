# Atividade_02 - Revisão de Conceitos. Livro AVR e Arduino – Técnicas de Projeto

> Capítulos: 4 (Manipulação de bits) e 5 (Portas)

Título: Conhecendo o Atmega328 e acessando portas de saída usando C/C++ / Debounce de botões e Encoders de posição

Objetivos: Entender a importância do debounce de botões e também o funcionamento de encoders como interface ao microcontrolador.

Nesta prática utilizaremos o Wokwi para simular um circuito simples usando o microcontrolador Atmega328p, utilizado nas placas Arduino UNO. Como entrada será utilizado um encoder rotativo e como saída 8 leds. O circuito base se encontra no link: https://wokwi.com/projects/373943840344172545

# Procedimentos:

1. Utilize o circuito fornecido para ativar os leds conforme pedido. Neste circuito os leds estão ligados ao PORTD e o encoder (nossa interface de entrada) ao PORTB. Note que os pinos PB0 e PB1 estão ligados aos pinos CLK e DT do encoder (responsáveis pela codificação de movimento rotatório) e o pino SW (chave central do encoder), ao pino PB2.

2. Para entender como o encoder funciona, visite o site https://www.allaboutcircuits.com/projects/how-to-use-a-rotary-encoder-in-a-mcu-based-project/ ou o site https://howtomechatronics.com/tutorials/arduino/rotary-encoder-works-use-arduino/ .


3. Programe para que o circuito execute o seguinte: ao girar a chave no sentido anti-horário, os leds devem se acender da direita para a esquerda. Ao girar a chave no sentido horário, os leds se apagam da direira para a esquerda, um de cada vez. Ao clicar na chave central, os leds acesos se apagam e os apagados se acendem.


Notem que os pinos CLK e SW devem ter implementado em suas rotinas o debounce. Já o pino DT não necessita de debounce. Isto ocorre porquê existe uma defasagem entre os pinos CLK e DT em 90 graus. Desta forma, quando ocorre o acionamento da chave ligada ao CLK, a chave ligada ao DT já estará num estado estável, podendo a sua leitura ser efetuada sem qualquer controle temporal, simplesmente lendo o registrador PINB na posição 1.

Vocês podem aproveitar o código de debounce fornecido (aqui).


4. EXTRA: Ao segurar o botão central por mais de 900ms, todos os leds se apagam.

5. Cole o código fonte do microcontrolador ao final deste arquivo.


**ATENÇÃO: Não utilize rotinas de interrupção. Não utilize pinMode, digitalWrite ou digitalRead. O uso destas funções acarretará na avaliação do trabalho com nota nula.**
