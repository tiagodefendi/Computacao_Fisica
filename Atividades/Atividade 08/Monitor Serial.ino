#define F_CPU 16000000UL  //define a frequencia do microcontrolador - 16MHz

#include <avr/io.h>       //definições do componente especificado
#include <util/delay.h>   //biblioteca para o uso das rotinas de _delay_ms e _delay_us()
#include <avr/pgmspace.h>   //para o uso do PROGMEM, gravação de dados na memória flash
#include <stdio.h>
#include <string.h>


#define BAUD   9600    //taxa de 2400 bps
#define MYUBRR  F_CPU/16/BAUD-1

// LEDs
#define LED13 PB5
#define LED12 PB4

// buffers circulares ==================================================================

// pos indica a posição corrente no buffer
// end indica a posição final no buffer com dados válidos
// note que end pode ser menor que pos, já que o buffer é circular
// cntbuff indica quantos caracteres válidos há no buffer

// Tamanho dos buffers
#define BUFFER_SIZE 16

// Buffer de transmissão, envia pelo terminal serial
char buffer_tx[BUFFER_SIZE];
short pos_tx=0,end_tx=0,cntbuffer_tx=0;

// Buffer de recepção, recebe pelo terminla serial
char buffer_rx[BUFFER_SIZE];
short pos_rx=0, end_rx=0, cntbuffer_rx=0;

// millis
char buffer_millis [9]; // buffer para guardar string do millis
unsigned long last_send = 0; // guadar o ultimo tempo q enviou a mensagem
unsigned char send_interval = 0; // guarda o ultimo intervalo de enviar a mensagem do millis (50 ou 10)

// configura serial ===================================================================

void USART_Inic(unsigned int ubrr0) {
  UBRR0H = (unsigned char)(ubrr0>>8); //Ajusta a taxa de transmissão
  UBRR0L = (unsigned char)ubrr0;

  UCSR0A = 0;//desabilitar velocidade dupla (no Arduino é habilitado por padrão)
  UCSR0B = (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0); //Habilita a transmissão e a recepção
  UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);/*modo assíncrono, 8 bits de dados, 1 bit de parada, sem paridade*/

  sei();
}

void configura_serial(unsigned int baud){
  unsigned long myubrr = F_CPU / 16 / baud - 1; // calcula o baud
  USART_Inic(myubrr); // inicializa a USART com o valor calculado
}

//---------------------------------------------------------------------------



// interrupção de transmissão
ISR(USART_UDRE_vect) {
  if (cntbuffer_tx>0) { // enquanto houver dados no buffer
    UDR0 = buffer_tx[pos_tx]; // envia
    pos_tx++;
    cntbuffer_tx--;
    pos_tx %= BUFFER_SIZE; // buffer circular
  } else { // se não houver o que enviar, desliga interrupção
    UCSR0B &= ~(1<<UDRIE0);
  }
}

// interrupção de recepção
ISR(USART_RX_vect) {
  char c = UDR0; // le o caractere recebido
  if (cntbuffer_rx < BUFFER_SIZE) { // se o buffer ainda não estiver cheio, armazena o caractere
    buffer_rx[end_rx] = c; // guarda o caractere na ultima posição
    end_rx++; // avança uma posição
    end_rx %= BUFFER_SIZE; // se passar do limite volta para primeira posição
    cntbuffer_rx++; // aumenta a quantidade de caracteres válidos
  }
}



// milis ==============================================================================

void send_millis() {
  sprintf(buffer_millis, "%lu\n", (10000 + (millis() % 90000))); // atualiza o millis
  send_tx(buffer_millis, strlen(buffer_millis)); // envia a mensagem de forma não bloqueante
}



// funções de tx =====================================================================

// envia mensagem para o terminal serial de forma não bloqueante
void send_tx(char* str, unsigned short n) {
  UCSR0B &= ~(1<<UDRIE0); // desabilita temporariamente as interrupções

  n %= BUFFER_SIZE; // volta para o inico se chegar no fim, ciclico

  // separa a cópia dos dados para o buffer em duas
  // partes se necessário for
  cntbuffer_tx += n;
  short cnt_temp = BUFFER_SIZE-end_tx;
  cnt_temp = cnt_temp>n?n:cnt_temp;;

  memcpy(buffer_tx+end_tx, str, cnt_temp);
  if (cnt_temp < n)
    memcpy(buffer_tx, str+cnt_temp, n-cnt_temp);

  end_tx += n;
  end_tx %= BUFFER_SIZE;
  // habilita interrupção para (re)iniciar o envio
  UCSR0B |= (1<<UDRIE0);
}



// funçoes de rx ======================================================================

// le o buffer de rx
char read_rx() { // lê caracter do buffer
  char temp; // caractere temporario
  if (!is_rx_empty()) { // verifica se não está vazio
    temp = buffer_rx[pos_rx]; // recebe o caractere do buffer
    pos_rx++; // avança posição
    pos_rx %= BUFFER_SIZE; // verifica se tem q voltar para o inicio, ciclico
    cntbuffer_rx--; // diminiu a quantidade de caracteres disponiveis
  }
  return temp;
}

// verifica se rx tá vazio
int is_rx_empty() {
    return !cntbuffer_rx;
}

// verifica as mesagens recebidas no serial em rx
void check_rx_msg() {
  char c = read_rx(); // le o caractere

  check_A13(c);
  check_S13(c);
  check_D12(c);
}

// verifica se a mensagem é D12
void check_D12(char c) {
  static int pos_d12 = 0; // indice para o caracter da mensagem de D12

  if (c=='D' && pos_d12 == 0){
    pos_d12++;
  }
  else if (c=='1' && pos_d12 == 1){
    pos_d12++;
  }
  else if (c=='2' && pos_d12 == 2){
    PORTB ^= (1 << LED12); // alterna o LED D12
    pos_d12 = 0;
  }
}

// verifica se a mensagem é testA
void check_A13(char c) {
  static int pos_a13 = 0; // indice para o caracter da mensagem de A13

  if (c=='A' && pos_a13 == 0){
    pos_a13++;
  }
  else if (c=='1' && pos_a13 == 1){
    pos_a13++;
  }
  else if (c=='3' && pos_a13 == 2){
    PORTB |= (1 << LED13); // liga LED A13
    pos_a13 = 0;
  }
}

// verifica se a mensagem é testS
void check_S13(char c) {
  static int pos_s13 = 0; // indice para o caracter da mensagem de D12

  if (c=='S' && pos_s13 == 0){
    pos_s13++;
  }
  else if (c=='1' && pos_s13 == 1){
    pos_s13++;
  }
  else if (c=='3' && pos_s13 == 2){
    PORTB &= ~(1 << LED13); // desliga LED S13
    pos_s13 = 0;
  }
}




// MAIN ================================================================================

int main() {
  init(); // começa o millis

  DDRB |= (1 << LED13) | (1 << LED12); // seta PB5 e PB4 como output

  configura_serial(2400); // inicializa a USART com o valor calculado para 2400 bps

  last_send = millis(); // ultimo envio
  send_interval = 50; // intervalo inicial: 50 ms

  while(1) { //laço infinito
    // PORTB |= (1 << LED13); // liga LED A13
    // PORTB &= ~(1 << LED13); // desliga o LED S13
    // PORTB ^= (1 << LED12); // alterna o LED D12

    if ((millis() - last_send) >= send_interval) { // verifica se passou o intervalo
      last_send = millis(); // atualiza o ultimo envio
      send_millis(); // envia a mensagem do millis

      // alterna o intervalo: se era 50ms, passa para 10ms; caso contrário, volta para 50ms.
      if (send_interval == 50)
        send_interval = 10;
      else
        send_interval = 50;
    }

    if (!is_rx_empty()) {
      check_rx_msg(); // verifica as mensagens recebidas pelo serial em rx
    }

    _delay_ms(5); // para não travar
  }
}
