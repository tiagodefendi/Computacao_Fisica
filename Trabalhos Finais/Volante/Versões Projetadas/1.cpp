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

  center_steering_wheel();
}

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

void loop() {
  //Move(100, false); // move ccw
  // Move(100); // move cw
  // debug only info
  if (millis()%300==0) {
    Serial.print(count);
    Serial.print(", ");
    Serial.println(absolute_sw==true?'1':'0');
  }
}

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

void center_steering_wheel() {
  unsigned long time = millis();
  bool control = true;

  // faz a primeira leitura do sw
  absolute_sw = 0 == (PINB&(1<<POS_SENSOR));

  // se estivar na chave tira da chave
  while(!absolute_sw) {
    Move(170, false);

    Serial.print(count);
    Serial.print(", ");
    Serial.println(absolute_sw==true?'1':'0');
  }
  // freia
  while((millis()-time) < 100) {
    Move(170, true);
  }
  Stop();
  Idle();

  // anda até chegar na chave
  while (absolute_sw) {
    Move(160, false);

    Serial.print(count);
    Serial.print(", ");
    Serial.println(absolute_sw==true?'1':'0');
  }

  // reseta o contador na chave
  count = 0;

  // freia
  while((millis()-time) < 100) {
    Move(170, true);
  }
  Stop();

  // calibra no meio
  while(!(count>190 && count<210)) {
    // move até chegar no centro
    while(!(count>190 && count<210)) {
      Move(160, control);

      Serial.print(count);
      Serial.print(", ");
      Serial.println(absolute_sw==true?'1':'0');
    }

    // altera o lado q o volante vai girar
    control = !control;

    // freia
    while((millis()-time) < 100) {
      Move(170, control);
    }
    Stop();

    // deley para analisar o onde está
    while (millis()-time<500) {

      Serial.print(count);
      Serial.print(", ");
      Serial.println(absolute_sw==true?'1':'0');
    }
  }

  //count = 0; // ajusta o centro

  Stop();
  Idle();

  // contar o tanto q andou e parar antes
}
