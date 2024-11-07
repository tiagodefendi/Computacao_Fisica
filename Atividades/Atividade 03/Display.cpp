#include <avr/pgmspace.h>

// Pinos dos displays
#define DIG1 PC3
#define DIG2 PC4
#define DIG3 PC5
#define DIG4 PC2
#define CLN PB3

// Pinos do encoder
#define CLK_PIN PB0
#define DT_PIN PB1
#define SW_PIN PB2

// Pino Buzzer
#define BUZZER PB4

// Tempos
#define DEBOUNCE_TIME 20
#define DIG_TIME 5
#define PRESS_TIME 750
#define ROLL_TIME 175
#define PULSE_OFF_TIME 300
#define PULSE_ON_TIME 800
#define CLN_TIME 500

#define TIMES_ADD 10

// Variáveis de controle
unsigned long lastupdate = 0;

unsigned long lastClkChangeTime = 0;
bool lastClkState = 0;

unsigned long lastButtonPressTime = 0;
bool lastButtonState = true;

unsigned long lastCountdownTime = 0;
bool countdownActive = false;

unsigned long buzzerStartTime = 0;
bool buzzerActive = false;

unsigned long lastCLNTime = 0;
bool lastCLNState = true;

bool pressing = false;

unsigned long pulseStart = 0;
bool pulsing = false;

bool settingSplitTimer = false;
bool minutes = true;

// Dígitos
char d = 0;
char digNumber;
char d0 = 0;
char d1 = 0;
char d2 = 0;
char d3 = 0;

const unsigned char Tabela[] PROGMEM = {
    0x40, // 0
    0x79, // 1
    0x24, // 2
    0x30, // 3
    0x19, // 4
    0x12, // 5
    0x02, // 6
    0x78, // 7
    0x00, // 8
    0x18, // 9
    0x08, // A
    0x03, // B
    0x46, // C
    0x21, // D
    0x06, // E
    0x0E  // F
};

// ------------------------------------------------------------------------------

void setup()
{
    // Configuração dos pinos do display
    DDRB = 0b11111110; // PB0 como pino de entrada, os demais pinos como saída
    PORTB = 0x01;      // Habilita o pull-up do PB0
    DDRD = 0xFF;       // PORTD como saída (display)
    PORTD = 0xFF;      // Desliga o display

    DDRC = 0xFF;  // Configura A2, A3, A4 e A5 como saídas para os dígitos
    PORTC = 0x00; // Desativa todos os dígitos no início

    // Configuração dos pinos do encoder
    DDRB &= ~((1 << DT_PIN) | (1 << CLK_PIN) | (1 << SW_PIN)); // Entradas com pull-up
    PORTB |= (1 << DT_PIN) | (1 << CLK_PIN) | (1 << SW_PIN);
    lastClkState = PINB & (1 << CLK_PIN); // Estado inicial do CLK

    // Configuração do pino do buzzer
    DDRB |= (1 << BUZZER);   // Configura o pino do buzzer como saída
    PORTB &= ~(1 << BUZZER); // Garante que o buzzer esteja desligado inicialmente

    DDRB |= (1 << CLN);   // Configura o pino do CLN como saída
    PORTB &= ~(1 << CLN); // Garante que o CLN esteja desligado inicialmente
}

void loop()
{
    bool currentClkState = PINB & (1 << CLK_PIN);
    bool currentDtState = PINB & (1 << DT_PIN);
    bool currentButtonState = PINB & (1 << SW_PIN);

    processEncoderRotate(currentClkState, currentDtState);
    processEncoderButtonPress(currentButtonState);

    if (buzzerActive && (millis() - buzzerStartTime >= 300))
    {
        PORTB &= ~(1 << BUZZER); // Desliga o buzzer
        buzzerActive = false;
    }

    showDisplay();
}

// --------------------------------------------------------------------------------------------

void showDisplay()
{
    if ((millis() - lastupdate) > DIG_TIME)
    {
        lastupdate = millis();

        d++;
        d %= 4; // Alterna entre os quatro dígitos

        PORTC = 0x00; // Desativa todos os dígitos antes de atualizar

        if (pressing)
        {
            displayPressing();
        }
        else if (settingSplitTimer)
        {
            displaySplit();
        }
        else
        {
            displayTogether();
        }
    }
}

void displaySplit()
{
    if (
        ((millis() - pulseStart) > PULSE_ON_TIME && !pulsing) ||
        ((millis() - pulseStart) > PULSE_OFF_TIME && pulsing))
    {
        pulseStart = millis();
        pulsing = !pulsing;
    }

    if (d == 0)
    {
        if (minutes && pulsing)
        {
            PORTC &= ~(1 << DIG1); // Desativa o pino A3 (DIG1)
        }
        else
        {
            digNumber = d0;
            PORTD = pgm_read_byte(&Tabela[digNumber]);
            PORTC |= (1 << DIG1); // Ativa o pino A3 (DIG1)
        }
    }
    else if (d == 1)
    {
        if (minutes && pulsing)
        {
            PORTC &= ~(1 << DIG2); // Desativa o pino A4 (DIG1)
        }
        else
        {
            digNumber = d1;
            PORTD = pgm_read_byte(&Tabela[digNumber]);
            PORTC |= (1 << DIG2); // Ativa o pino A4 (DIG2)
        }
    }
    else if (d == 2)
    {
        if (!minutes && pulsing)
        {
            PORTC &= ~(1 << DIG3); // Desativa o pino A5 (DIG1)
        }
        else
        {
            digNumber = d2;
            PORTD = pgm_read_byte(&Tabela[digNumber]);
            PORTC |= (1 << DIG3); // Ativa o pino A5 (DIG3)
        }
    }
    else if (d == 3)
    {
        if (!minutes && pulsing)
        {
            PORTC &= ~(1 << DIG4); // Desativa o pino A2 (DIG1)
        }
        else
        {
            digNumber = d3;
            PORTD = pgm_read_byte(&Tabela[digNumber]);
            PORTC |= (1 << DIG4); // Ativa o pino A2 (DIG4)
        }
    }
}

void displayTogether()
{
    if (d == 0)
    {
        digNumber = d0;
        PORTD = pgm_read_byte(&Tabela[digNumber]);
        PORTC |= (1 << DIG1); // Ativa o pino A3 (DIG1)
    }
    else if (d == 1)
    {
        digNumber = d1;
        PORTD = pgm_read_byte(&Tabela[digNumber]);
        PORTC |= (1 << DIG2); // Ativa o pino A4 (DIG2)
    }
    else if (d == 2)
    {
        digNumber = d2;
        PORTD = pgm_read_byte(&Tabela[digNumber]);
        PORTC |= (1 << DIG3); // Ativa o pino A5 (DIG3)
    }
    else if (d == 3)
    {
        digNumber = d3;
        PORTD = pgm_read_byte(&Tabela[digNumber]);
        PORTC |= (1 << DIG4); // Ativa o pino A2 (DIG4)
    }
}

void displayPressing()
{
    if (d == 0)
    {
        PORTD = pgm_read_byte(&Tabela[0]);
        PORTC |= (1 << DIG1); // Ativa o pino A3 (DIG1)
    }
    else if (d == 1)
    {
        PORTD = pgm_read_byte(&Tabela[12]);
        PORTC |= (1 << DIG2); // Ativa o pino A4 (DIG2)
    }
    else if (d == 2)
    {
        PORTD = pgm_read_byte(&Tabela[13]);
        PORTC |= (1 << DIG3); // Ativa o pino A5 (DIG3)
    }
    else if (d == 3)
    {
        PORTD = pgm_read_byte(&Tabela[0]);
        PORTC |= (1 << DIG4); // Ativa o pino A2 (DIG4)
    }
}

// -----------------------------------------------------------------------------------------

void processEncoderRotate(bool currentClkState, bool currentDtState)
{
    if (currentClkState != lastClkState && (millis() - lastClkChangeTime) > DEBOUNCE_TIME)
    {
        unsigned long timeSinceLastChange = millis() - lastClkChangeTime;

        lastClkChangeTime = millis();
        lastClkState = currentClkState;

        // Define giro rápido ou lento com base no tempo decorrido
        if (timeSinceLastChange > ROLL_TIME)
        { // Giro lento
            timeChange(currentClkState, currentDtState);
        }
        else
        { // Giro rapido
            timeChangeFast(currentClkState, currentDtState);
        }
    }
}

void processEncoderButtonPress(bool currentButtonState)
{
    if (currentButtonState != lastButtonState && (millis() - lastClkChangeTime) > DEBOUNCE_TIME)
    {
        lastClkChangeTime = millis();

        if (!currentButtonState)
        {
            lastButtonPressTime = millis();

            while (!currentButtonState)
            {
                if (millis() - lastButtonPressTime > PRESS_TIME)
                    pressing = true;
                showDisplay();
                currentButtonState = PINB & (1 << SW_PIN); // Aguarda liberação
            }
            pressing = false;
        }

        unsigned long pressDuration = millis() - lastButtonPressTime; // Calcula tempo do clique

        if (!settingSplitTimer)
        {
            processTogether(pressDuration);
        }
        else
        {
            processSplit(pressDuration);
        }

        lastButtonState = currentButtonState;
    }
}

// -------------------------------------------------------------------------------------------

void processTogether(unsigned long pressDuration)
{
    if (pressDuration < PRESS_TIME)
    { // Clicar no botão
        run_timer(false);
    }
    else
    { // Segurar botão
        settingSplitTimer = true;
    }
}

void processSplit(unsigned long pressDuration)
{
    if (pressDuration < PRESS_TIME)
    { // Clicar no botão
        minutes = !minutes;
        // trocar entre minutos e segundos
    }
    else
    { // Segurar botão
        settingSplitTimer = false;
    }
}

// ---------------------------------------------------------------------------------------------

void timeChange(bool currentClkState, bool currentDtState)
{
    if (!settingSplitTimer)
    { // troca normal
        if (currentClkState == 0)
        {
            if (currentDtState != 0)
            { // Anti-horário
                decrementDisplayValues();
            }
            else
            { // Horário
                incrementDisplayValues();
            }
        }
    }
    else
    { // troca para quando está separado
        if (minutes)
        { // altera minuto
            if (currentClkState == 0)
            {
                if (currentDtState != 0)
                { // Anti-horário
                    decrementMinutesDisplayValues();
                }
                else
                { // Horário
                    incrementMinutesDisplayValues();
                }
            }
        }
        else
        { // altera segundo
            if (currentClkState == 0)
            {
                if (currentDtState != 0)
                { // Anti-horário
                    decrementSecondsDisplayValues();
                }
                else
                { // Horário
                    incrementSecondsDisplayValues();
                }
            }
        }
    }
}

void timeChangeFast(bool currentClkState, bool currentDtState)
{
    if (!settingSplitTimer)
    { // troca normal
        if (currentClkState == 0)
        {
            if (currentDtState != 0)
            { // Anti-horário
                decrementDisplayValues();
                decrementDisplayValues();
                decrementDisplayValues();
                decrementDisplayValues();
                decrementDisplayValues();
            }
            else
            { // Horário
                incrementDisplayValues();
                incrementDisplayValues();
                incrementDisplayValues();
                incrementDisplayValues();
                incrementDisplayValues();
            }
        }
    }
    else
    { // troca para quando está separado
        if (minutes)
        { // altera minuto
            if (currentClkState == 0)
            {
                if (currentDtState != 0)
                { // Anti-horário
                    decrementMinutesDisplayValues();
                    decrementMinutesDisplayValues();
                    decrementMinutesDisplayValues();
                    decrementMinutesDisplayValues();
                    decrementMinutesDisplayValues();
                }
                else
                { // Horário
                    incrementMinutesDisplayValues();
                    incrementMinutesDisplayValues();
                    incrementMinutesDisplayValues();
                    incrementMinutesDisplayValues();
                    incrementMinutesDisplayValues();
                }
            }
        }
        else
        { // altera segundo
            if (currentClkState == 0)
            {
                if (currentDtState != 0)
                { // Anti-horário
                    decrementSecondsDisplayValues();
                }
                else
                { // Horário
                    incrementSecondsDisplayValues();
                }
            }
        }
    }
}

void incrementDisplayValues()
{
    if (d0 == 5 && d1 == 9 && d2 == 5 && d3 == 9)
    {
        return;
    }

    if (++d3 > 9)
    {
        d3 = 0;
        if (++d2 > 5)
        { // Limita segundos (0-59)
            d2 = 0;
            if (++d1 > 9)
            {
                d1 = 0;
                if (++d0 > 5)
                { // Limita minutos (0-59)
                    d0 = 0;
                }
            }
        }
    }
}

void incrementMinutesDisplayValues()
{
    if (d0 == 5 && d1 == 9)
    {
        return;
    }

    if (++d1 > 9)
    {
        d1 = 0;
        if (++d0 > 5)
        { // Limita minutos (0-59)
            d0 = 0;
        }
    }
}

void incrementSecondsDisplayValues()
{
    if (d2 == 5 && d3 == 9)
    {
        return;
    }

    if (++d3 > 9)
    {
        d3 = 0;
        if (++d2 > 5)
        { // Limita segundos (0-59)
            d2 = 0;
        }
    }
}

void decrementDisplayValues()
{
    if (d0 == 0 && d1 == 0 && d2 == 0 && d3 == 0)
    {
        return;
    }

    if (--d3 < 0)
    {
        d3 = 9;
        if (--d2 < 0)
        {
            d2 = 5; // Limita segundos (0-59)
            if (--d1 < 0)
            {
                d1 = 9;
                if (--d0 < 0)
                {
                    d0 = 5; // Limita minutos (0-59)
                }
            }
        }
    }
}

void decrementMinutesDisplayValues()
{
    if (d0 == 0 && d1 == 0)
    {
        return;
    }

    if (--d1 < 0)
    {
        d1 = 9;
        if (--d0 < 0)
        {
            d0 = 5; // Limita minutos (0-59)
        }
    }
}

void decrementSecondsDisplayValues()
{
    if (d2 == 0 && d3 == 0)
    {
        return;
    }

    if (--d3 < 0)
    {
        d3 = 9;
        if (--d2 < 0)
        {
            d2 = 5; // Limita segundos (0-59)
        }
    }
}

// -------------------------------------------------------------------------------------------

void run_timer(bool currentButtonState)
{
    if (!currentButtonState)
    { // Ativa a contagem regressiva ao pressionar o botão
        countdownActive = true;
        lastCountdownTime = millis(); // Define o tempo inicial para a contagem

        while (countdownActive)
        {
            if ((millis() - lastCLNTime) > CLN_TIME)
            { // pontin do meio
                lastCLNTime = millis();
                if (lastCLNState)
                {
                    PORTB |= (1 << CLN); // desliga os pontinhos
                    lastCLNState = !lastCLNState;
                }
                else
                {
                    PORTB &= ~(1 << CLN); // liga eles
                    lastCLNState = !lastCLNState;
                }
            }

            if (countdownActive && ((millis() - lastCountdownTime) > 1000))
            { // Decremento a cada segundo
                lastCountdownTime = millis();

                decrementDisplayValues();

                if (d0 == 0 && d1 == 0 && d2 == 0 && d3 == 0)
                {
                    countdownActive = false; // Para o contador quando atinge zero
                    buzzerOn();
                }
            }

            showDisplay();
        }

        PORTB &= ~(1 << CLN);
    }
}

// -----------------------------------------------------------------------------------------------

void buzzerOn()
{
    PORTB |= (1 << BUZZER); // Liga o buzzer
    buzzerActive = true;
    buzzerStartTime = millis(); // Armazena o tempo em que o buzzer foi ligado
}

void resetDisplayValues()
{
    d0 = d1 = d2 = d3 = 0;
}

// github.com/tiagodefendi
