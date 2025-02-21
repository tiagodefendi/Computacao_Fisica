#define CLK_PIN PB0
#define DT_PIN PB1
#define SW_PIN PB2
#define LED_COUNT 8
#define DEBOUNCE_TIME 10

char ledCounter = 0;
bool lastClkState = 0;
long lastClkChangeTime = 0;
long lastButtonPressTime = 0;
bool lastButtonState = true;

void setup() {
    DDRB &= ~((1 << DT_PIN) | (1 << CLK_PIN) | (1 << SW_PIN)); // Entradas com pull-up
    PORTB |= (1 << DT_PIN) | (1 << CLK_PIN) | (1 << SW_PIN);
    lastClkState = PINB & (1 << CLK_PIN);

    DDRD = 255; // Saídas para LEDs
    PORTD = 0;
}

void loop() {
    bool currentClkState = PINB & (1 << CLK_PIN);
    bool currentDtState = PINB & (1 << DT_PIN);
    bool currentButtonState = PINB & (1 << SW_PIN);

    processEncoderRotate(currentClkState, currentDtState);
    processEncoderButtonPress(currentButtonState);
}

void processEncoderRotate(bool currentClkState, bool currentDtState) {
    if (currentClkState != lastClkState && (millis() - lastClkChangeTime) > DEBOUNCE_TIME) {
        lastClkChangeTime = millis();

        if (currentClkState == 0) {
            if (currentDtState != 0) { // Anti-horário
                if (ledCounter > 0) {
                    PORTD &= ~(1 << (ledCounter - 1));
                    ledCounter--;
                }
            } else { // Horário
                if (ledCounter < LED_COUNT) {
                    PORTD |= (1 << ledCounter);
                    ledCounter++;
                }
            }
        }
        lastClkState = currentClkState;
    }
}

void processEncoderButtonPress(bool currentButtonState) {
    if (currentButtonState != lastButtonState && (millis() - lastClkChangeTime) > DEBOUNCE_TIME) {
        lastClkChangeTime = millis();

        if (!currentButtonState) lastButtonPressTime = millis();

        if (currentButtonState && (millis() - lastButtonPressTime) < 900) {
            PORTD ^= 255; // Inverte LEDs
        }

        lastButtonState = currentButtonState;
    }

    if (!currentButtonState && (millis() - lastButtonPressTime) > 900) {
        PORTD = 0;
        ledCounter = 0;
        while (!currentButtonState) currentButtonState = PINB & (1 << SW_PIN); // Aguarda liberação
    }
}

// github.com/tiagodefendi
