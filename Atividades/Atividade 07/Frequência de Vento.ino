#include <stdio.h>

unsigned long lastLCDRefresh = 0;

#define DADOS_LCD      PORTD   //4 bits de dados do LCD no PORTD
#define nibble_dados  1   //0 para via de dados do LCD nos 4 LSBs do PORT empregado (Px0-D4, Px1-D5, Px2-D6, Px3-D7)
                //1 para via de dados do LCD nos 4 MSBs do PORT empregado (Px4-D4, Px5-D5, Px6-D6, Px7-D7)
#define CONTR_LCD     PORTB   //PORT com os pinos de controle do LCD (pino R/W em 0).
#define E         PB4     //pino de habilitação do LCD (enable)
#define RS        PB3     //pino para informar se o dado é uma instrução ou caractere

//sinal de habilitação para o LCD
#define pulso_enable()  _delay_us(1); CONTR_LCD|=(1<<E); _delay_us(1); CONTR_LCD&=~(1<<E); _delay_us(45)

//protótipo das funções
void cmd_LCD(unsigned char c, char cd);
void inic_LCD_4bits();

//---------------------------------------------------------------------------------------------
// Sub-rotina para enviar caracteres e comandos ao LCD com via de dados de 4 bits
//---------------------------------------------------------------------------------------------
void cmd_LCD(unsigned char c, char cd)        //c é o dado  e cd indica se é instrução ou caractere
{
  if(cd==0)
    CONTR_LCD&=~(1<<RS);
  else
    CONTR_LCD|=(1<<RS);

  //primeiro nibble de dados - 4 MSB
  #if (nibble_dados)                //compila código para os pinos de dados do LCD nos 4 MSB do PORT
    DADOS_LCD = (DADOS_LCD & 0b00001111) | (0b11110000 & c);
  #else                     //compila código para os pinos de dados do LCD nos 4 LSB do PORT
    DADOS_LCD = (DADOS_LCD & 0xF0)|(c>>4);
  #endif

  pulso_enable();

  //segundo nibble de dados - 4 LSB
  #if (nibble_dados)                //compila código para os pinos de dados do LCD nos 4 MSB do PORT
    DADOS_LCD = (DADOS_LCD & 0b00001111) | (0b11110000 & (c<<4));
  #else                     //compila código para os pinos de dados do LCD nos 4 LSB do PORT
    DADOS_LCD = (DADOS_LCD & 0xF0) | (0x0F & c);
  #endif

  pulso_enable();

  if((cd==0) && (c<4))        //se for instrução de retorno ou limpeza espera LCD estar pronto
    _delay_ms(2);
}
//---------------------------------------------------------------------------------------------
//Sub-rotina para inicialização do LCD com via de dados de 4 bits
//---------------------------------------------------------------------------------------------
void inic_LCD_4bits()   //sequência ditada pelo fabricando do circuito integrado HD44780
{             //o LCD será só escrito. Então, R/W é sempre zero.

  CONTR_LCD&=~(1<<RS);  //RS em zero indicando que o dado para o LCD será uma instrução
  CONTR_LCD&=~(1<<E); //pino de habilitação em zero

  _delay_ms(20);      //tempo para estabilizar a tensão do LCD, após VCC ultrapassar 4.5 V (na prática pode
              //ser maior).

  cmd_LCD(0x30,0);

  pulso_enable();     //habilitação respeitando os tempos de resposta do LCD
  _delay_ms(5);
  pulso_enable();
  _delay_us(200);
  pulso_enable(); /*até aqui ainda é uma interface de 8 bits.
          Muitos programadores desprezam os comandos acima, respeitando apenas o tempo de
          estabilização da tensão (geralmente funciona). Se o LCD não for inicializado primeiro no
          modo de 8 bits, haverá problemas se o microcontrolador for inicializado e o display já o tiver sido.*/

  //interface de 4 bits, deve ser enviado duas vezes (a outra está abaixo)
  cmd_LCD(0x20,0);

  pulso_enable();
    cmd_LCD(0x28,0);    //interface de 4 bits 2 linhas (aqui se habilita as 2 linhas)
              //são enviados os 2 nibbles (0x2 e 0x8)
    cmd_LCD(0x08,0);    //desliga o display
    cmd_LCD(0x01,0);    //limpa todo o display
    cmd_LCD(0x0C,0);    //mensagem aparente cursor inativo não piscando
    cmd_LCD(0x80,0);    //inicializa cursor na primeira posição a esquerda - 1a linha
}

//Sub-rotina de escrita no LCD -  dados armazenados na RAM
void escreve_LCD(char *c)
{
   for (; *c!=0;c++) cmd_LCD(*c,1);
}

unsigned long long mymillis;

// =============================================================================

// botões
#define SENSOR PD2
#define READ_SENSOR PIND & (1 << SENSOR)
#define PROP PB0
#define READ_PROP PINB & (1 << PROP)
#define RPM PB1
#define READ_RPM PINB & (1 << RPM)

// tempos
#define DEBOUNCE 50
#define SCREEN_UPDATE 100 // tempo para atualizar a tela
#define MINMS 9 // a cada 10ms acrescenta o tempo do relogio

char fline[17]; // primeira linha
char sline[17]; // segunda linha

unsigned long last_update = 0; // ultima atualizacao da freq
double freq = 0; // guarda a frequencia
unsigned char prop = 1; // guarda a quantidade de helice(min 1, max 9)

// variaveis para o debounce do botao das helices
unsigned char prop_last_state;
unsigned long prop_last_change = 0;

bool rpm = false; // verifica se é hz ou rpm
// variaveis para o debounce do botao do rpm
unsigned char rpm_last_state;
unsigned long rpm_last_change = 0;

unsigned long cycle = 0; // ultimo tempo
unsigned long last_cycle = 0; // tempo anterior
int counter = 0; // contador para usar na frequencia

// =============================================================================

void setup(){
  // lcd output
  DDRB |= 0b00011000;
  DDRD |= 0b11110000;

  // Configura o sensor de rotações
  DDRD &= ~(1 << SENSOR);
  PORTD |= (1 << SENSOR);

  // Configura hélice
  DDRB &= ~(1 << PROP);
  PORTB |= (1 << PROP);
  prop_last_state = READ_PROP;

  // Configura botão de trocar unidade
  DDRB &= ~(1 << RPM);
  PORTB |= (1 << RPM);
  rpm_last_state = READ_RPM;

  // inicializa LCD
  inic_LCD_4bits();

  // TODO: enable external interrups
  // falling edge

  ADCSRA |= (1<<ADIE); // habilita IE
  ADCSRA |= (1<<ADEN); // habilita o ADC
  EIMSK |= (1 << INT0); // IE no INT0
  EICRA |= (1 << ISC01) | (1 << ISC00);  // borda de subida
}

void loop(){
  // TODO: ler botao

  read_rpm(); // ve se apertou botao rpm
  read_prop(); // ve se apertou botao das helices
  update_freq(); // atualiza a frequencia

  if (mymillis!=millis()) {
  	mymillis=millis();
  }
  if (millis()>(lastLCDRefresh+SCREEN_UPDATE)) {
    lastLCDRefresh = millis();
    cmd_LCD(0x80, 0);

    // TODO: atualizar display

  	update_screen(); // atualiza a tela
  }

  delay(1); // Only for simulation
}

// interrupt handler
ISR(INT0_vect){
  // TODO: what to do?

  counter++; // soma a frequencia
  last_cycle = cycle; // guarda o ultimo tempo
  cycle = millis(); // atualiza o tempo
}

void update_screen(){
  reset_fline(); // reseta a primeira linha

  double temp = freq; // guarda a frequencia
  if (rpm) {
    temp = hz_to_rpm(); // atualiza para rpm
  }

  long temp_int = (long)temp; // parte inteira
  long temp_dec = (long)(temp * 10) % 10; // decimal

  if (rpm) { // string diferente para rpm ou hz
    sprintf(fline, "   %d.%dRPM", temp_int, temp_dec);
  }
  else {
    sprintf(fline, "   %d.%dHz", temp_int, temp_dec);
  }

  sprintf(sline, "Helices: %d", prop); // string da helice

  cmd_LCD(0x80,0); // cursor na primeira linha
  escreve_LCD(fline); // escreve a string da frequencia
  cmd_LCD(0xC0,0); // cursor na segunda linha
  escreve_LCD(sline); // escreve a string das helices
}

// resta a primeira linha
void reset_fline(){
  char temp[] = "                "; // seta uma string limpa
  cmd_LCD(0x80,0); // seta primeira linha
  escreve_LCD(temp); // usa a string para limpar a primeira linha
}

// converte hz para rpm
float hz_to_rpm(){
  return freq*60; // 1Hz = 60RPM
}

// le o botao de helice
void read_prop(){
  unsigned char prop_state = READ_PROP; // estado atual
  if (prop_last_state != prop_state && millis() - prop_last_change > DEBOUNCE) { // debounce
    prop_last_change = millis();

    if (!prop_state) { // se tiver precionado e incrementa
      if (++prop > 9){ // quando passar de nove volta para 1
        prop = 1;
      }
    }

    prop_last_state = prop_state;
  }
}

// le a frequencia
bool update_freq() {
    if (millis() - last_update > SCREEN_UPDATE) { // verifica se passou 100ms
      last_update = millis();

      if(cycle - last_cycle > 10){ // verifica a diferença
        freq = 1000 / (cycle - last_cycle);
      }else{
        freq = counter * 10; // multiplica por 10 a contagem nos 100ms
      }
      counter = 0; // reseta a contagem
      freq /= prop; // divide a frequencia pela quantidade de helice
    }
}

// verifica se trocou o rpm
void read_rpm(){
  unsigned char rpm_state = READ_RPM; // estado atual
  if (rpm_last_state != rpm_state && millis() - rpm_last_change > DEBOUNCE) { // debounce
    rpm_last_change = millis();

    if (!rpm_state) { // se tiver precionado
      rpm = !rpm; // troca de hz para rpm e vice e versa
    }

    rpm_last_state = rpm_state;
  }
}
