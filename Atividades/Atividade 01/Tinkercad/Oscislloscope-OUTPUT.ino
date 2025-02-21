void setup(){
    DDRB |= (1<<PB5);
}

void loop(){
    PORTB |= (1<<PB5);
    delay(50);
    PORTB &= ~(1<<PB5);
    delay(50);

// github.com/tiagodefendi
