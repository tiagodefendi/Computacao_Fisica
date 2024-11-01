void setup(){
    pinMode(27, OUTPUT);
    pinMode(23, OUTPUT);
}

void loop(){
    digitalWrite(27, HIGH);
    delay(50);
    digitalWrite(27, LOW);
    digitalWrite(23, HIGH);
    delay(50);
    digitalWrite(23, LOW);
    delay(450);
}

// github.com/tiagodefendi
