unsigned char d=0;
unsigned long lastDispRefresh = 0;
// Tabela = {0,1,2,3,4,5,6,7,8,9,E,r}
unsigned char Tabela[] = {0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x80, 0x18, 0x06, 0x2F};
unsigned char todisp[3] = {10, 11, 11};

char ch = 0;

// CONFIGURACOES ==============================================================================

#define MIN 90 // valor minimo com folga
#define MAX 410 // valor maximo com folga

unsigned char percent[2];
int value[2];
char avg;

unsigned char channel = 0;

// MAIN =======================================================================================


void setup(){
  DDRD |= 0b01111111;
  DDRB |= 0b00000111;
  
  DDRC &= ~(1<<PC0); // PC0 as input
  DDRC &= ~(1<<PC1); // PC1 as input
  
  ADMUX &= ~((1<<REFS0)|(1<<REFS1)); // tensao de referencia
  ADMUX |= (1<<REFS0); // AVCC
  
  ADCSRB = 0; // valor padrão
  
  ADCSRA &= 0b11111000;
  ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); // ADCCLK = CLK/128
  
  ADCSRA |= (1<<ADIE); // habilita interrupção
  ADCSRA |= (1<<ADEN); // habilita o ADC
  ADCSRA |= (1<<ADSC); // primeira conversão do ADC
}

void loop(){
  // read the analog in value:
  //value[0] = ler_adc(0);
  //value[1] = ler_adc(1);
  
  // map it to the range of the analog out: 
  percent[0] = map(value[0], MIN*2, MAX*2, 0, 100); // calcula porcentagem
  percent[1] = map(value[1], MIN, MAX, 0, 100); // calcula porcentagem
  
  if (abs(percent[0] - percent[1]) < 10) { // se a diferença é menor q 10
    avg = (percent[0] + percent[1]) / 2; // calcula a media
    
    // verifica o limite inferior e superior ja que deixou uma folguinha
    if (avg < 0) { // se der um menor q 0 limita em 0
      avg = 0;
    }
    else if (avg > 100) { // se der um maior q 100 limita em 100
      avg = 100;
    }
    
    // seta o valor do display
    set_val(avg);
  }
  else {
    // escreve erro na tela
    set_error();
  }
  
  // verificando a porcentagem ja e possivel identificar os cabos desconectados
  // salvo algumas execoes q sao identificadas nesse if
  // esse if verifica quando todos os cabos estão desconectados e os sinais ficam como 3
  // ou ainda quando os pinos a esquerda são os unicos ligados q os sinais são 3
  // ou ainda quando os do meio são os unicos conectados e manda o sinal 10
  // ou quando o da esquerda é o unico desconectado e o sinal é 1023
  // esse if detecta se algum sinal se encaixa como desconctado e seta erro
  // usei o MIN-30 como folga ja q teoricamente o min e o menor valor possivel
  if(value[0] < MIN-30 || value[1] < MIN-30) {
    set_error();
  }
  
  
  // Serial.begin(115200);
  // Serial.println(value[0]);
  // Serial.println(value[1]);
  // Serial.println(percent_rp);
  // Serial.println(percent_rp);
  // Serial.println(abs(percent_rp - percent_lp));
  // Serial.println(avg);
  // Serial.end();

  delay(1); // Only for simulation
}

// busy waiting version
signed int ler_adc(unsigned char canal){
  ADMUX &= 0b11110000;
  ADMUX |= (0b00001111&canal); // seleciona o canal ch no MUX
  DIDR0 = (1<<canal);
  
  ADCSRA |= (1<<ADSC);       //inicia a conversão
  
  while(ADCSRA & (1<<ADSC));  //espera a conversão ser finalizada
  
  return ADC;
}

// interrupt handler
ISR(ADC_vect){
  // TODO:
  
  // multiplexar o chaveamento dos displays dentro da rotina de interrupção do ADC
  // e viavel visto q a conversao nao e bloqueante q so ocorre depois de uma conversao
  // alem de q o uso do if verificando o tempo da ultima interacao nao fazer o display funcionar alem do necessario
  // o q acarretaria em um tempo de vida menor e maior gasto de enrgia
  if (millis()>(lastDispRefresh+4)) {
    lastDispRefresh = millis();
  	d++;
    d%=3;
    
    PORTB = (PORTB&0b11111000)|(1<<d); // ativa o display correspondente ao d
    PORTD = Tabela[todisp[d]];
  }
  
  // read the analog in value:
  if (channel) {
    value[1] = ADC; // guarda ADC1 resultado
  } else {
    value[0] = ADC; // guarda ADC0 resultaado
  }
  
  // troca de canal
  channel = (channel + 1) % 2;
  ADMUX = (ADMUX & 0b11110000) | channel;
  
  ADCSRA |= (1<<ADSC); // proxima conversão
}

// seta os numeros do display
void set_val(char val){
  todisp[0] = val/100;
  todisp[1] = (val%100)/10;
  todisp[2] = val%10;
}

// seta erro no display
void set_error(){
  todisp[0] = 10;
  todisp[1] = 11;
  todisp[2] = 11;
}
