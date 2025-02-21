#include "LCD.h" // LCD

// temperature related
#define MAX_TEMP  212.f
#define MIN_TEMP  11.f
#define MAX_COUNT 1011
#define MIN_COUNT 79
//212 1011
//190 912
//160 771
//138 670
//120 585
//29 154
//11 79

unsigned int adc_res; // temperatura

// --------------------
//Variables
float temperature_read = 0.0f;

#define num_ADC_average 32
volatile unsigned int adc_result0[num_ADC_average]; // average 32 conversions channel 0
volatile unsigned int adc_pos0 = num_ADC_average-1; // current ADC value for channel 0

volatile unsigned long my_millis = 0;

#define MOTOR PD2
#define HEAT PD3
#define BUZZER PC1

#define MOTORON   PORTD |= 1<<MOTOR
#define MOTOROFF  PORTD &= ~(1<<MOTOR)

#define HEATON   PORTD |= 1<<HEAT
#define HEATOFF  PORTD &= ~(1<<HEAT)

#define BUZZON   PORTC |= 1<<BUZZER
#define BUZZOFF  PORTC &= ~(1<<BUZZER)
#define BUZZCOM PORTC ^= (1<<BUZZER)

#define COUNT2TEMP(c)  (MIN_TEMP+(((MAX_TEMP-MIN_TEMP)/(MAX_COUNT-MIN_COUNT))*(((float)(c))-MIN_COUNT)))

volatile unsigned int beep_period;

#define BEEP()  beep_period = 200

// ============================================================================================

// daley para o tinkercad
#define DELAY 5

// tempo
#define TIME 200 // 1min = 60 s / 1s = 1.000ms -> 1min = 60 x 1.000 = 60.000ms

// pinos
#define MODE PC2
#define READ_MODE PINC & (1 << MODE)

#define ADD PC3
#define READ_ADD PINC & (1 << ADD)

#define SUB PC4
#define READ_SUB PINC & (1 << SUB)

#define START PC5
#define READ_START PINC & (1 << START)

// botões e debounce
#define DEBOUNCE 50

bool last_state_mode = 0;
unsigned long last_change_mode = 0;
unsigned char control_mode = 0;

bool last_state_add = 0;
unsigned long last_change_add = 0;

bool last_state_sub = 0;
unsigned long last_change_sub = 0;

bool last_state_start = 0;
unsigned long last_change_start = 0;

// strings
char str_sova[17] = "    > SOVA <    ";
int time_sova = 25; // tempo em minutos / padrão 25

char str_crescimento[17] = "> CRESCIMENTO < ";
int time_crescimento = 90; // tempo em minutos / padrão 90 = 1:30

char str_assadura[17] = "  > ASSADURA <  ";
int time_assadura = 40; // tempo em minutos / padrão 40

// formato da string "TM:XX:XX TP:XX.X"
// char str_second_line[17] = "TM:00:00 TP:00.0";
char str_time[6] = "XX:XX";
char str_temp[5] = "XX.X";

unsigned int time_count = 0;
unsigned long last_change_time;


// MAIN ============================================================================================

int main() {
  init();
  //void setup() {
  // disable interrupts
  cli();

  MOTOROFF;
  HEATOFF;
  BUZZOFF;
  
  ADMUX = 0; // ADC0, AREF
  //ADMUX = (1 << REFS0); //selecionei o VCC como referência, se não ficava travado em 214 graus
  ADCSRA = 1<<ADEN|1<<ADIE;
  ADCSRA |= 1<<ADPS2|1<<ADPS1|1<<ADPS0; // 128 ADC prescaler / 9615 conversions/second
  ADCSRB = 0;
  DIDR0 = 1<<ADC0D; // disable digital input on A0

  // configuracao timer3
  // use mode 0 normal
  TCCR1A = 0;
  TCCR1B = (1<<CS11); // clkio/8 prescaler
  TCCR1C = 0;
  OCR1A = 0x07CF; //1999 that counts 2000 = 1ms
  TIMSK1 = 1<< OCIE1A; // output compare unit A

  DDRD = 0xFF; // LCD e Motor e Resistencia
  DDRB = 1<<PB0|1<<PB1; // LCD
  DDRC = 1<<PC1; // BUZZER

  // botoes
  DDRC &= ~(1 << PC2 | 1 << PC3 | 1 << PC4 | 1 << PC5);
  PORTC |= (1 << PC2 | 1 << PC3 | 1 << PC4 | 1 << PC5);
  //verifica o estado inicial dos botoes
  last_state_mode = READ_MODE;
  last_state_add = READ_ADD;
  last_state_sub = READ_SUB;
  last_state_start = READ_START;

  // enable interrupts
  sei();

  Serial.begin(115200);

  ADCSRA |= 1<<ADSC; // start ADC conversion

  inic_LCD_4bits();

  // BEEP();
  
  cmd_LCD(0x80,0); // desloca para primeira posicao
  escreve_LCD(str_sova); // primeiro modo sova
  cmd_LCD(0xC0,0); //desloca cursor para a segunda linha
  escreve_LCD("TM:00:00 TP:00.0"); //string de tempo e temperatura
  update_time(time_sova); // ajusta para o tempo da sova
  

  while(1) {
    update_temp(); // atualiza a temperatura constantemente
    
    read_mode(); // le o botao de troca de modo
    read_add(); // le o botao de adicionar tempo
    read_sub(); // le o botao de remover tempo
    read_start(); // le o botao de inicio do processo
    
    _delay_ms(DELAY); // para não travar o tinkercad
    
  }//while 1

  return 0;
}

ISR (ADC_vect)
{
  adc_result0[adc_pos0++%num_ADC_average] = ADC;
  ADCSRA |= 1<<ADSC; // start new ADC conversion  
}

ISR(TIMER1_COMPA_vect)
{
  OCR1A += 0x07CF;
  my_millis += 1;

  if (beep_period > 0) {
    beep_period--;
    BUZZCOM;
  }
}

// TELA =======================================================================

void update_mode(char* str) {
  cmd_LCD(0x80,0); // desloca para primeira posicao
  escreve_LCD(str); // escreve a string na primeira linha
}

void update_time(int time) {
  unsigned char hours = time / 60; // sepera as horas
  unsigned char minutes = time % 60; // separa os minutos
  
  sprintf(str_time, "%.2d:%.2d", hours, minutes); // ajusta string de tempo
  
  cmd_LCD(0xC3,0); //desloca cursor para a segunda linha depois de TM:
  escreve_LCD(str_time); // atualiza na tela o tempo
}

// atualiza a temperatura
void update_temp() {
  //sprintf(str_temp, "%.4d", temperature_read); // atualiza string de temperatura
  dtostrf(temperature_read, 4, 1, str_temp);
  cmd_LCD(0xCC,0); //desloca cursor para a segunda linha tepois de TP:
  escreve_LCD(str_temp); // atualiza no led a temperatura
}

// CHANGE MODE =============================================================================

// faz leitura do botao de troca de modo
void read_mode() {
  bool state_mode = READ_MODE; // verifica estado atual
  
  // debounce
  if ((last_state_mode != state_mode) && (millis() - last_change_mode > DEBOUNCE)) {
    last_state_mode = state_mode; // atualiza ultimo estado
    last_change_mode = millis(); // atualiza ultima troca
    
    if (!state_mode) { // se estiver pressionado
      change_mode(); // troca de modo
    }
  }
}

// troca o modo no sistema e na tela
void change_mode() {
  control_mode++; // passa para o proximo modo
  control_mode %= 3; // volta para o primeiro modo quando passar por todos
  
  // atualiza a tela para o modo atual
  switch(control_mode){
    case 0: // primeiro modo sova
      update_mode(str_sova);
      update_time(time_sova);
      break;

    case 1: // crescimento
      update_mode(str_crescimento);
      update_time(time_crescimento);
      break;

    case 2: // por fim assadura
      update_mode(str_assadura);
      update_time(time_assadura);
      break;
  }
}

// ADD TIME ====================================================================

// leitura do botão para aumentar o tempo
// o tempo maximo e de 99hrs e 59 min na nossa implementacao, caso a pessoa queira torrar o pao ela pode kkkk
void read_add() {
  // define o fator de adicao (se a pessoa clicar muito rapido soma 15 min de uma vez)
    unsigned char sum_factor = (millis() - last_change_add<100)? 15 : 1;
  
  bool state_add = READ_ADD; // verifica o estado atual
  
  // debounce
  if ((last_state_add != state_add) && (millis() - last_change_add > DEBOUNCE)) {
    last_state_add = state_add; // atualiza ultimo estado
    last_change_add = millis(); // atualiza ultima troca
    
    if (!state_add) { // se o botão estiver pressionado
      switch(control_mode){ // switch case para saber em qual modo aumentar o tempo
        case 0: // primeiro caso é a sova
          if (time_sova + sum_factor < 5999) { // verifica se a soma não vai passar o limite
            time_sova += sum_factor; // soma e atualiza
            update_time(time_sova);
          }
          // essa verificação está aq pois se o fator for grande ainda pode ter tempo para aumentar até o limite
          else if (time_sova + 1 > 5999) { // verifica se nao vai ultrapasar o limite
            time_sova++; // soma e atualiza
            update_time(time_sova);
          }
          break;
        
        // mesma logica pra todos outros modos
        case 1: // crescimento
          if (time_crescimento + sum_factor < 5999) {
            time_crescimento += sum_factor;
            update_time(time_crescimento);
          }
          else if (time_crescimento + 1 > 5999) {
            time_crescimento++;
            update_time(time_crescimento);
          }
          break;

        case 2: // modo de assadura
          if (time_assadura + sum_factor < 5999) {
            time_assadura += sum_factor;
            update_time(time_assadura);
          }
          else if (time_assadura + 1 > 5999) {
            time_assadura++;
            update_time(time_assadura);
          }
          break;
      }
    }
  }
}

// SUB TIME =======================================================================

// leitura do botão para diminuir o tempo
// o tempo pode ser 0 na nossa implementacao, caso a pessoa queira pular um processo
void read_sub() {
  // define o fator de subtração (se a pessoa clicar muito rapido diminui 15 min de uma vez)
  unsigned char sub_factor = (millis() - last_change_sub<100)? 15 : 1;
  
  bool state_sub = READ_SUB; // pega o estado atual
  
  // debounce
  if ((last_state_sub != state_sub) && (millis() - last_change_sub > DEBOUNCE)) {
    last_state_sub = state_sub; // atualiza ultimo estado
    last_change_sub = millis(); // atualiza ultima troca
    
    
    if (!state_sub) { // se o botão for pressionado
      switch(control_mode){ // switch case para saber em qual modo diminuir o tempo
        case 0: // primeiro caso e na etapa de sova
          if (time_sova - sub_factor > -1) { // verifica se o numero nao ira ficar negativo
            time_sova -= sub_factor; // diminui e atualiza o tempo
            update_time(time_sova);
          }
          // essa verificação está aq pois se o fator for grande ainda pode ter tempo para reduzir
          else if (time_sova - 1 > -1) { // verifica se nao vai ficar negativo
            time_sova--; // diminui e atualiza
            update_time(time_sova);
          }
          break;
        
        // mesma coisa para todos outros casos
        case 1: // caso do crescimento
          if (time_crescimento - sub_factor > -1) {
            time_crescimento -= sub_factor;
            update_time(time_crescimento);
          }
          else if (time_crescimento - 1 > -1) {
            time_crescimento--;
            update_time(time_crescimento);
          }
          break;

        case 2: // caso da assadura
          if (time_assadura - sub_factor > -1) {
            time_assadura -= sub_factor;
            update_time(time_assadura);
          }
          else if (time_assadura - 1 > -1) {
            time_assadura--;
            update_time(time_assadura);
          }
          break;
      }
    }
  }
}

// CLOCK ====================================================================================

// calcula o tempo do cronometro
void run_time(unsigned int time) {
  if (millis() - last_change_time >= TIME) { // verifica se ja se passou o intervalo de um minuto
    last_change_time = millis(); // atualiza ultima troca
    time_count++; // adiciona um minuto no contador
    
    update_time(time - time_count); // atualiza tempo na tela
  }
}

// START ====================================================================================

// le o botão de começar
void read_start() {
  bool state_start = READ_START; // pega o estado atual
  
  // debounce
  if ((last_state_start != state_start) && (millis() - last_change_start > DEBOUNCE)) {
    last_state_start = state_start; // atualiza o ultimo estado
    last_change_start = millis(); // atualiza ultima troca
    
    if (!state_start) { // caso o botão esteja pressionado
    	bake(); // assa o pao
    }
  }
}

// rotina para assar o pao
void bake() {
  // meramente cosmetico
  update_mode("PADARIA DO FRANK"); // escreve na primeira linha
  cmd_LCD(0xC0,0); //desloca cursor para a segunda linha
  escreve_LCD("VAMOS COZINHAR! "); // escreve na segunda linha
  
  _delay_ms(1000); // delay cosmetico kkkkk
  
  cmd_LCD(0xC0,0); //desloca cursor para a segunda linha
  escreve_LCD("TM:00:00 TP:00.0"); // formato o tempo e temperatura
  
  
  // sova --------------------------------------------------------
  update_mode("    SOVANDO     "); // atualiza modo atual
  update_time(time_sova); // atualiza o tempo
  
  MOTORON; // ligao o motor
  
  last_change_time = millis(); // atualiza ultima mudança de tempo para iniciar o cronometro
  while(time_count < time_sova) { // deixa sovando até acabar o tempo de sova
    run_time(time_sova); // atualiza o tempo
    update_temp(); // atualiza temperatura
    
    _delay_ms(DELAY); // para não travar o tinkercad
  }
  
  MOTOROFF; // desliga o motor
  
  time_count = 0; // reseta o tempo
  
  
  // crescimento ------------------------------------------------------
  update_mode("   CRESCENDO    "); // atualiza modo atual
  update_time(time_crescimento); // atualiza o tempo
  
  last_change_time = millis(); // atualiza ultima mudança de tempo para iniciar o cronometro
  while(time_count < time_crescimento) { // deixa crescer até acabar o tempo de crescimento
    run_time(time_crescimento); // atualiza o tempo
    update_temp(); // atualiza temperatura
    
    _delay_ms(DELAY); // para não travar o tinkercad
  }
  
  time_count = 0; // reseta o tempo
    
  
  // assadura --------------------------------------------------------
  update_mode("    ASSANDO     "); // atualiza modo atual
  update_time(time_assadura); // atualiza o tempo
  
  HEATON; // liga a resistencia para assar o pão
  
  last_change_time = millis(); // atualiza ultima mudança de tempo para iniciar o cronometro
  while(time_count < time_assadura) { // deixa crescer até acabar o tempo de assadura
    run_time(time_assadura); // atualiza o tempo
    update_temp(); // atualiza temperatura
    
    _delay_ms(DELAY); // para não travar o tinkercad
  }
  
  HEATOFF; // desliga a resistencia
  
  time_count = 0; // reseta o tempo
  
  
  BEEP(); // apita quando terminar o processo
  
  // volta para tela inicial
  control_mode = 2;
  change_mode();
}
