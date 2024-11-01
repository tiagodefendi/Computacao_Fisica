void setup(){
    DDRB |= (1<<PB2);
    DDRD |= (1<<PD5);
}

void loop(){
    PORTB |= (1<<PB2);
    delay(50);
    PORTB &= ~(1<<PB2);
    PORTD |= (1<<PD5);
    delay(50);
    PORTD &= ~(1<<PD5);
    delay(450);
}

// github.com/tiagodefendi
