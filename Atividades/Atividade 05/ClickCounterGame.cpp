// CODIGO DO PROFESSOR ===========================================================

#ifndef _DEF_PRINCIPAIS_H
#define _DEF_PRINCIPAIS_H

#define F_CPU 16000000UL  //define a frequencia do microcontrolador - 16MHz

#include <avr/io.h>       //definições do componente especificado
#include <util/delay.h>   //biblioteca para o uso das rotinas de _delay_ms e _delay_us()
#include <avr/pgmspace.h>   //para o uso do PROGMEM, gravação de dados na memória flash

//Definições de macros para o trabalho com bits

#define set_bit(y,bit)  (y|=(1<<bit)) //coloca em 1 o bit x da variável Y
#define clr_bit(y,bit)  (y&=~(1<<bit))  //coloca em 0 o bit x da variável Y
#define cpl_bit(y,bit)  (y^=(1<<bit)) //troca o estado lógico do bit x da variável Y
#define tst_bit(y,bit)  (y&(1<<bit))  //retorna 0 ou 1 conforme leitura do bit

#endif


#ifndef _LCD_H
#define _LCD_H

//Definições para facilitar a troca dos pinos do hardware e facilitar a re-programação

#define DADOS_LCD      PORTC  //4 bits de dados do LCD no PORTD 
#define nibble_dados  0   //0 para via de dados do LCD nos 4 LSBs do PORT empregado (Px0-D4, Px1-D5, Px2-D6, Px3-D7) 
                //1 para via de dados do LCD nos 4 MSBs do PORT empregado (Px4-D4, Px5-D5, Px6-D6, Px7-D7) 
#define CONTR_LCD     PORTC   //PORT com os pinos de controle do LCD (pino R/W em 0).
#define E         PC4     //pino de habilitação do LCD (enable)
#define RS        PC5     //pino para informar se o dado é uma instrução ou caractere

#define tam_vetor 5 //número de digitos individuais para a conversão por ident_num()   
#define conv_ascii  48  //48 se ident_num() deve retornar um número no formato ASCII (0 para formato normal)

//sinal de habilitação para o LCD
#define pulso_enable()  _delay_us(1); set_bit(CONTR_LCD,E); _delay_us(1); clr_bit(CONTR_LCD,E); _delay_us(45)

//protótipo das funções
void cmd_LCD(unsigned char c, char cd);
void inic_LCD_4bits();    
void escreve_LCD(char *c);
void escreve_LCD_Flash(const char *c);

void ident_num(unsigned int valor, unsigned char *disp);

#endif

// Sub-rotina para enviar caracteres e comandos ao LCD com via de dados de 4 bits
void cmd_LCD(unsigned char c, char cd)        //c é o dado  e cd indica se é instrução ou caractere
{
  if(cd==0)
    clr_bit(CONTR_LCD,RS);
  else
    set_bit(CONTR_LCD,RS);

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

//Sub-rotina para inicialização do LCD com via de dados de 4 bits
void inic_LCD_4bits()   //sequência ditada pelo fabricando do circuito integrado HD44780
{             //o LCD será só escrito. Então, R/W é sempre zero.

  clr_bit(CONTR_LCD,RS);  //RS em zero indicando que o dado para o LCD será uma instrução 
  clr_bit(CONTR_LCD,E); //pino de habilitação em zero
  
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

//Sub-rotina de escrita no LCD - dados armazenados na FLASH
void escreve_LCD_Flash(const char *c)
{
   for (;pgm_read_byte(&(*c))!=0;c++)
     cmd_LCD(pgm_read_byte(&(*c)),1);
}

//Conversão de um número em seus digitos individuais
void ident_num(unsigned int valor, unsigned char *disp)
{   
  unsigned char n;

  for(n=0; n<tam_vetor; n++)
    disp[n] = 0 + conv_ascii;   //limpa vetor para armazenagem do digitos 

  do
  {
       *disp = (valor%10) + conv_ascii; //pega o resto da divisao por 10 
     valor /=10;            //pega o inteiro da divisão por 10
     disp++;

  }while (valor!=0);
}

//definiçao para acessar a memória flash 
PROGMEM const char mensagem[] = " DADOS DE 4BITS!\0"; //mensagem armazenada na memória flash
// =============================================================================





/*
PROFESSOR EU FIZ UMAS COISINHAS A MAIS ESPERO Q VC NAO SE IMPORTE ;p
 - CONTAGEM REGRESSIVA
 - TELA DE FIM
 - AJUSTE NO ESPACO EM BRANCO (ENTRE O A: E SEUS PONTOS) ESTAVA ME INCOMODANDO kkkkkk
 	(A: 00   B:00) -> (A:00    B:00)
*/





// CONFIGURACOES INICIAIS ======================================================

// botoes
// direita
#define RB PB1 // Right Button
#define READ_RB PINB & (1 << RB)
// esquerda
#define LB PB0 // Left Button
#define READ_LB PINB & (1 << LB)

// tempos
#define DEBOUNCE 50
#define MATCH_DURATION 500 // em milisegundos dividido por 10: 15000ms / 10 -> 1500 -> 15s
#define MINMS 9 // a cada 10ms acrescenta o tempo do relogio

// tempo da partida
unsigned long last_time_change = 0; // guarda a ultima mudança de tempo
// duracao total da partida q não pode ser maior q 9m59se99ms 
unsigned int match_duration = (MATCH_DURATION < 59999)? MATCH_DURATION : 59999;
unsigned int time_now = 0; // tempo atual da partida
bool match_finished = false;

// tela
char points[17]; // primeira linha
char time[17]; // segunda linha
// atualizar a tela em regioes especificas
char only_time[8]; // 
char score_rb[3]; // ponto string direita
char score_lb[3]; // ponto string esquerda

// botao direito
bool last_state_rb;
unsigned long last_change_rb = 0;
unsigned int rb_points = 0;
// botao esquerdo
bool last_state_lb;
unsigned long last_change_lb = 0;
unsigned int lb_points = 0;

// botao reset
bool last_state_reset;
unsigned long last_change_reset = 0;



// MAIN ========================================================================

void setup() {
  DDRC = 0xFF; //PORTC como saída
  
  // botao direito
  DDRB &= ~(1 << RB); // entrada
  PORTB |= (1 << RB); // saida
  last_state_rb = READ_RB;
  // botao esquerdo
  DDRB &= ~(1 << LB); // entrada
  PORTB |= (1 << LB); // saida
  last_state_rb = READ_LB;
  
  inic_LCD_4bits(); //inicializa o LCD
  
  // contagem regressiva
  sprintf(points, "   PARTIDA EM   ");
  sprintf(time, "       3        ");
  show_screen();
  delay(1000);
  sprintf(time, "       2        ");
  show_screen();
  delay(1000);
  sprintf(time, "       1        ");
  show_screen();
  delay(1000);
  sprintf(time, "       JA       ");
  show_screen();
  delay(333);
  
  points_to_string(); // coloca os pontos no placar
  time_to_string(match_duration); // coloca o tempo da tela certo
  show_screen(); // ajusta tela para o inicio
}

void loop(){
  // roda o jogo
  if(!match_finished) {
  	run_timer(); // ajusta o tempo
    read_buttons(); // le os botoes
  }
  
  delay(1); // delay para não travar
}



// SCREEN ======================================================================
// atualiza a tela inteira com as duas strings principais de tamanho 16 (points, time)
void show_screen() {
  cmd_LCD(0x80,0); // desloca para primeira posicao
  escreve_LCD(points); //string de pontos
  cmd_LCD(0xC0,0); //desloca cursor para a segunda linha
  escreve_LCD(time); //string de tempo
}

// atualiza apenas as regioes onde ocorrem mudancas (pontos e placar)
// atualiza o tempo
void update_time(){
  cmd_LCD(0xC4,0); //desloca no tempo
  escreve_LCD(only_time); //string de tempo
}
// atualiza os pontos
void update_rb_score(){ // direita
  points_to_string(); // transforma em string
  cmd_LCD(0x8C,0); // desloca para após A:
  escreve_LCD(score_rb); //coloca os pontos
}
void update_lb_score(){ // esquerda
  points_to_string(); // transforma em string
  cmd_LCD(0x84,0); // desloca para após B:
  escreve_LCD(score_lb); //coloca os pontos
}

// mostra o resultado da partida
void show_final_score(){
  sprintf(time, " TEMPO ESGOTADO ", rb_points);
  show_screen();

  delay(1000);

  if (rb_points > lb_points) {
    sprintf(points, "  VENCEDOR: B   ");
    sprintf(time, "   %.2d PONTOS    ", rb_points);
  }
  else if (rb_points < lb_points) {
    sprintf(points, "  VENCEDOR: A   ");
    sprintf(time, "   %.2d PONTOS    ", lb_points);
  }
  else {
    sprintf(points, "     EMPATE     ");
    sprintf(time, "   %.2d PONTOS    ", rb_points);
  }
  show_screen();
}



// POINTS ======================================================================
// le os botoes
void read_buttons() {
  read_rb();
  read_lb();
}

// le botao direito
void read_rb() {
  bool state_rb = READ_RB; 
  if (last_state_rb != state_rb && millis() - last_change_rb > DEBOUNCE) { // debounce
    last_change_rb = millis();

    if (!state_rb && rb_points<99) { // se tiver precionado e for menor q 99 incrementa
      rb_points++;      
      update_rb_score(); // atualiza no placar
    }
    
    last_state_rb = state_rb;
  }
}

// le botao esquerdo
void read_lb() {
  bool state_lb = READ_LB; 
  if (last_state_lb != state_lb && millis() - last_change_lb > DEBOUNCE) { // debounce
    last_change_lb = millis();

    if (!state_lb && lb_points<99) { // se tiver precionado e for menor q 99 incrementa
      lb_points++;
      update_lb_score(); // atualiza no placar
    }
    
    last_state_lb = state_lb;
  }
}

// atualiza o placar com os pontos respectivo de cada botao
void points_to_string() {
  sprintf(points, "  A:%.2d    B:%.2d  ", lb_points, rb_points);
  sprintf(score_rb, "%.2d", rb_points);
  sprintf(score_lb, "%.2d", lb_points);
}



// TIME ========================================================================
// roda o cronômetro
void run_timer() {
  if (time_now < match_duration) {
    
    sum_time(); // acrescenta o tempo
    time_to_string(match_duration-time_now); // transforma em string
    update_time(); // atualiza no display
    
    if(time_now == match_duration) { // termina a partida quando acabar o tempo
      match_finished = true;
      read_buttons(); // le os botoes
      show_screen(); // mostra a tela
      show_final_score(); // mostra o placar final
    }
  }
}

// acrescenta o tempo decorrido de partida
void sum_time() {
  if (millis() - last_time_change > MINMS) {
    last_time_change = millis();
    time_now ++;
  }
}

// atualiza o tempo da tela
void time_to_string(unsigned int tm){
  unsigned int temp = tm; // guarda o tempo temporariamente

  // minuto
  unsigned char min = temp / 6000;
  temp %= 6000;
  // dezena do segundo
  unsigned char s = temp / 1000;
  temp %= 1000;
  // unidade do segundo
  s *= 10;
  s += temp / 100;
  temp %= 100;
  // primeiro digito dos mili segundos
  unsigned char ms = temp / 10;
  temp %= 10;
  // segundo digito dos mili segundos
  ms *= 10;
  ms += temp;
  
  sprintf(time, "    %d:%.2d.%.2d     ", min, s, ms);
  sprintf(only_time, "%d:%.2d.%.2d", min, s, ms);
}
