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

void move_from_key() {
  unsigned long time = millis();

  if (!absolute_sw) {
    // se estivar na chave tira da chave
    while(!absolute_sw) {
      Move(165, false);

      Serial.print(count);
      Serial.print(", ");
      Serial.println(absolute_sw==true?'1':'0');
    }
    // freia
    while((millis()-time) < 100) {
      Move(165, true);
    }
    time = millis();
    Stop();
  }
}

void move_to_key() {
  unsigned long time = millis();

  // anda até chegar na chave
  while (absolute_sw) {
    if (!absolute_sw) {
      // reseta o contador na chave
      count = 0;
    }

    Move(160, false);

    Serial.print(count);
    Serial.print(", ");
    Serial.println(absolute_sw==true?'1':'0');

  }

  // freia
  while((millis()-time) < 100) {
    Move(160, true);
  }
  time = millis();
  Stop();
}

void try_center(long distance, int power) {
  unsigned long time = millis();

  // move até chegar no centro
  while(!(count-distance>540 && count-distance<560)) {
    if (count-distance<540) {
      Move(power, true);
    }
    if (count-distance>560) {
      Move(power, false);
    }

    Serial.print(count);
    Serial.print(", ");
    Serial.println(absolute_sw==true?'1':'0');
  }

  if (!(count-distance>540 && count-distance<560)) {
    // freia
    while((millis()-time) < 100) {
      if (count-distance<540) {
        Move(power-5, true);
      }
      if (count-distance>560) {
        Move(power-5, false);
      }
    }
    time = millis();
  }

  if (count-distance>540 && count-distance<560) {
    time = millis();
    // segura centralizadp
    while((millis()-time) < 500) {
      Move(power-5, true);
      Move(power-5, false);
    }
    time = millis();
  }

  Stop();
}

void center(long distance, int power) {
  unsigned long time = millis();
  int i = 0;

  // calibra no meio
  while(!(count-distance>540 && count-distance<560)) {
    try_center(power, (160-i));

    // deley para analisar o onde está
    while (millis()-time < 1000) {

      Serial.print(count);
      Serial.print(", ");
      Serial.println(absolute_sw==true?'1':'0');
    }
    time = millis();

    i += 5;
  }
}

void center_steering_wheel() {
  // faz a primeira leitura do sw
  absolute_sw = 0 == (PINB&(1<<POS_SENSOR));

  move_from_key();

  move_to_key();

  long distance = count;

  center(distance);

  count = 0; // ajusta o centro

  Stop();
  Idle();
  /*
  */
}
