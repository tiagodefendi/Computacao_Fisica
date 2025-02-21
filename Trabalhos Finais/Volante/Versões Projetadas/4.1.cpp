#define ATUA_CW PB4
#define ATUA_CCW PB5 // HIGH aqui gira CCW
#define ATUA_STRENGTH PB3

#define ROTARY_ENC_A 6
#define ROTARY_ENC_B 7
#define ROTARY_ENC_PCINT_A PCINT22
#define ROTARY_ENC_PCINT_B PCINT23
#define ROTARY_ENC_PCINT_AB_IE PCIE2

#define POS_SENSOR PB2 // switch (absolute position)
#define GP_BUTTON PB0 // general purpose button

#define CENTER_CW 600 // chave até o centro sentido horário
#define CENTER_CCW 280 // chave até o centro sentido anti horário

#include "Rotary.h"
volatile long count = 0; // encoder_rotativo = posicao relativa depois de ligado
volatile bool absolute_sw = false; // chave de posicao do volante ativa?

Rotary r = Rotary(ROTARY_ENC_A, ROTARY_ENC_B);

// DEFINES ================================================================================================

// count update
int last_count;

// valor da posição das chaves, foi verificado manualmente
#define INI_POS_CENTER_CCW 550
#define END_POS_CENTER_CCW 720
#define INI_POS_CENTER_CW 2850
#define END_POS_CENTER_CW 3050

// posição do centro usada
int center = INI_POS_CENTER_CCW;

// quanto de espaço foi deixado para margem
#define GAP 5

// força padrão de giro
#define INI_POWER 155
#define GAIN_RATE 1.005
#define REDUCE_RATE 0.99

// energua necessária para mover 1 grau
#define ENERGY 1
// quantidade de count para dar uma volta
#define CIRCUMFERENCE 3600
// quantas voltas são necessárias para travar as rodas
#define TURNS 2.5
// quantidade max de graus q as rodas se movem
#define MAX_DEGREES 180

// count totoal para q trave as rodas
int count_total = (int)(CIRCUMFERENCE * TURNS);
// quantidade de graus o servo se move q representa o movimento de um count do volante
float relative_dregrees = MAX_DEGREES/count_total;

// SETUP =========================================================================================

void setup() {
  Serial.begin(115200);
  r.begin(true);
  PCICR |= (1 << ROTARY_ENC_PCINT_AB_IE);
  PCMSK2 |= (1 << ROTARY_ENC_PCINT_A) | (1 << ROTARY_ENC_PCINT_B);

  DDRB &= ~((1<<GP_BUTTON)|(1<<POS_SENSOR));
  DDRB |= (1<<ATUA_CW)|(1<<ATUA_CCW)|(1<<ATUA_STRENGTH);

  PORTB |= (1<<POS_SENSOR);
  PORTB |= (1<<GP_BUTTON);
  PORTB &= ~(1<<ATUA_CW);
  PORTB &= ~(1<<ATUA_CCW);

  initPWM();
  sei();

  Idle();
  absolute_sw = 0==(PINB&(1<<POS_SENSOR));

  leave_key(); // caso esteja na chave sai dela a direita
  // locate_key(); // procura a posição da chave
  // center_steering_wheel(); // centraliza o volante apos ligar o circuito e encontrar a chave
}

// FUNCOES =========================================================================================

void initPWM() {
  OCR2A = 0;
  TCCR2A = (1<<WGM20);
  TCCR2B = (1<<CS21);
  TCCR2A |= (1<<COM2A1);
}

void setPWM(unsigned char val) {
  OCR2A = val<200?val:200;
}

void Stop() {
  PORTB |= (1<<ATUA_CW);
  PORTB |= (1<<ATUA_CCW);
}

void Idle() {
  setPWM(0);
  PORTB &= ~(1<<ATUA_CW);
  PORTB &= ~(1<<ATUA_CCW);
}

void Move(unsigned char power, bool cw = true) {
  if (power == 0)
    Idle();
  else {
    if (cw) {
      PORTB |= (1<<ATUA_CW);
      PORTB &= ~(1<<ATUA_CCW);
    } else {
      PORTB &= ~(1<<ATUA_CW);
      PORTB |= (1<<ATUA_CCW);
    }
    setPWM(power);
  }
}

int dir = ATUA_CCW;

// LOOP =========================================================================================

void loop() {
  //Move(100, false); // move ccw
  // Move(100); // move cw
  // debug only info
  if (millis()%300==0) {
    Serial.print(count);
    Serial.print(", ");
    Serial.println(absolute_sw==true?'1':'0');
  }

  // move_servo();
}

// INTERRUPCAO =========================================================================================

ISR(PCINT2_vect) {
  unsigned char result = r.process();
  if (result == DIR_NONE) {
    // do nothing
  }
  else if (result == DIR_CW) {
    count--;
  }
  else if (result == DIR_CCW) {
    count++;
  }

  absolute_sw = 0==(PINB&(1<<POS_SENSOR));
}

// CENTRALIZAR =========================================================================================

// função para aumentar a força do volante para conseguir tirar ele da inercia
int gain_power(int power) {
    last_count = count; // guarda posiçãoa atual

    unsigned long time = millis();
    while(millis() - time < 300) { // tenta se mover
        Move(power);
    }

    if (count - last_count < 20) { // caso não se mova
        while(count - last_count < 20) { // aumeta força até se mover
            Move((int)power);
            power *= GAIN_RATE;
        }
    }

    return (int)power;
}

// função para tirar a força do volante de forma mais suave
inline int reduce_power(int power) {
    return (int)(power*REDUCE_RATE);
}

// sair da chave pela direita quando estiver nela --------------------------
void leave_key() {
  unsigned long time = millis();

  // faz a primeira leitura do sw
  absolute_sw = 0 == (PINB&(1<<POS_SENSOR));

  if (!absolute_sw) {
    // se estivar na chave tira da chave
    while(!absolute_sw) {
      Move(170); // move para direita para sair da chave

      Serial.print(count);
      Serial.print(", ");
      Serial.println(absolute_sw==true?'1':'0');
    }
    Stop(); // freia
  }
}

// encontrar o inicio da chave pelo sentido anti-horario --------------------------
void locate_key() {
  unsigned long time = millis();

  int power = INI_POWER; // coloca força inicial
  power = gain_power(power); // ajusta força para sair da inercia

  // anda até chegar no inicio da chave pela esquerda
  while (absolute_sw) {
    if (!absolute_sw) {
      count = 0; // reseta o contador na chave
    }

    power = gain_power(power); // ajusta força para sair da inercia

    Move(power, false); // anda para esquerda até achar a chave

    Serial.print(count);
    Serial.print(", ");
    Serial.println(absolute_sw==true?'1':'0');
  }
  Stop();

  // freia
  while((millis()-time) < 100) {
    Move(power+5, true);
  }
  Stop();

//   count = 0; // reseta o contador na chave

  center += count; // ajusta posição do centro
}

// centralizar o volante
void center_steering_wheel() {
  unsigned long time = millis();
  bool control = true;

  int power = INI_POWER; // coloca força inicial
  power = gain_power(power); // ajusta força para sair da inercia

  // calibra no meio
  while(!(count>(center-GAP) && count<(center+GAP))) {
    // move até chegar no centro
    while(!(count>(center-GAP) && count<(center+GAP))) {
      if (!(count>(center-GAP) && count<(center+GAP))) {
        Move(power, control);
        power = gain_power(power); // ajusta força para sair da inercia
      }

      Serial.print(count);
      Serial.print(", ");
      Serial.println(absolute_sw==true?'1':'0');
    }
    Stop();

    if (!(count>(center-GAP) && count<(center+GAP))) {
      // freia
      while((millis()-time) < 100) {
        if (!(count>(center-GAP) && count<(center+GAP))) {
          Move(power, !control);
        }
      }
      Stop();
    }

    // altera o lado q o volante vai girar
    control = !control;

    // deley para analisar o onde está
    time = millis();
    while (millis()-time < 333) {

      Serial.print(count);
      Serial.print(", ");
      Serial.println(absolute_sw==true?'1':'0');
    }
    time = millis();

    power = reduce_power;
  }
  Stop();

  count = 0; // ajusta o centro
  last_count = count; // ajusta a ultima posição para o centro

  Stop();
  Idle();
}

// centralizar o volante
void center_steering_wheel2() {
  unsigned long time = millis();
  bool control = true;

  int power = INI_POWER; // coloca força inicial
  power = gain_power(power); // ajusta força para sair da inercia

  // calibra no meio
  while(!(count>(center-GAP) && count<(center+GAP))) {
      if (!(count>(center-GAP) && count<(center+GAP))) {
        power = gain_power(power); // ajusta força para sair da inercia
      }

      // move para direita até o volante chegar no centro
      while(count<(center-GAP)) {
        Move(power, true);
      }
      // freio
      // Move(power+10, false);
      Stop();

      power = reduce_power(power); // reduz velocidade na proxima interação

      // move para esquerda até o volante chegar no centro
      while(count>(center-GAP)) {
        Move(power, false);
      }
      // freio
      // Move(power+10, true);
      Stop();


      Serial.print(count);
      Serial.print(", ");
      Serial.println(absolute_sw==true?'1':'0');
  }
  Stop();
  Idle();

  // center_steering_wheel2(); // chama função para garantir q vai ficar no meio

  count = 0; // ajusta o centro
  last_count = count; // ajusta a ultima posição para o centro
}

// MOVE SERVO =========================================================================================

float calc_energy(int movement_distance) {
  return ENERGY * movement_distance * relative_dregrees;
}

void sent_energy_to_servo(int energy, bool direction = true) {
  if (direction) {
    // mandar energia e rodar para a direita
  }
  else {
    // mandar energia e rodar para a esquerda
  }
}

void move_servo() {
  // trava quando dar as duas voltas e meia, uma volta é 3600 então 2,5 é 9000
  if (count >= -count_total && count <= count) { // 3600*2.5
    if (count - last_count >= 10) { // verifica se andou para direita
      sent_energy_to_servo(calc_energy(count-last_count)); // manda energia suficiente para mover o grau relativo 180/9000=0.02, o 180 é devido q o servo deve andar 180 graus até travar
      last_count = count; // atualiza o movimento atual
    }
    if (count - last_count <= -10) { // verifica se andou para esquerda
      sent_energy_to_servo(calc_energy(count-last_count), false); // manda energia suficiente para mover o grau relativo 180/9000=0.02, o 180 é devido q o servo deve andar 180 graus até travar
      last_count = count; // atualiza o movimento atual
    }
  }
}
