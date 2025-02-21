/*
preciso multplexar o display para  q ele coloque os numeros um de cada vez
*/

#include <avr/io.h> 	 	//definições do componente especificado
#include <util/delay.h>		//biblioteca para o uso das rotinas de _delay_
#include <avr/pgmspace.h>	//biblioteca para poder gravar dados na memória flash

//Definições de macros - para o trabalho com os bits de uma variável
#define tst_bit(Y,bit_x) (Y&(1<<bit_x)) //testa o bit x da variável Y (retorna 0 ou 1)
#define clr_bit(Y,bit_x) (Y&=~(1<<bit_x))
#define set_bit(Y,bit_x) (Y|=(1<<bit_x))

#define LINHA PIND //registrador para a leitura das linhas
#define COLUNA PORTD //registrador para a escrita nas colunas

// led
#define LED PB5

// digitos
#define DIG1 PB1
#define DIG2 PB2
#define DIG3 PB3
#define DIG4 PB4

// times
#define DEBOUNCE 20

// =======================================================================

/*
matriz com as informações para decodificação do teclado,
organizada de acordo com a configuração do teclado, o usuário
pode definir valores números ou caracteres ASCII, como neste exemplo
*/
const unsigned char teclado[4][4] PROGMEM = {
 {'1', '2', '3', 'A'},
 {'4', '5', '6', 'B'},
 {'7', '8', '9', 'C'},
 {'*', '0', '#', 'D'}
};

// digitos display 7 segmentos
const unsigned char Tabela[] PROGMEM = {
  0x40,
  0x79,
  0x24,
  0x30,
  0x19,
  0x12,
  0x02,
  0x78,
  0x80,
  0x18,
  0x08,
  0x03,
  0x46,
  0x21,
  0x06,
  0x0E
};

char coluna = 0;
unsigned long last_column_change = 0;

// verificacao do debounce
unsigned char last_change_state[4];
unsigned long last_change_time = 0;

bool locked = false; // define se o cofre esta bloqueado ou nao

unsigned char idx = 0; // posicao da senha
unsigned char password[4]; // senha do cofre
unsigned char password_attempt[4]; // guarda a tentativa de acesso da senha



// MAIN CODE =======================================================================

void setup(){
  // Serial.begin(9600);
  // Serial.println("Setup"); // indica q começou o código

  DDRB = 0xFF; //saida
  DDRD = 0x0F; //definições das entradas e saídas para o teclado
  PORTD= 0xFF; //habilita os pull-ups do PORTD e coloca colunas em 1
  UCSR0B = 0x00; //para uso dos PORTD no Arduino

  DDRC = 0xFF;
  PORTC = 0xFF;
  PORTB |= 0x01;

  PORTB |= 0b00011111;

  last_column_change = millis();

  set_password();
}

void loop(){
  press_key();

  if (idx == 4) {
    check_password(); // verifica se a senha esta correta
  }

  // anda pelas colunas
  column_change();

  display_led(); // verfica o estado do cofre e liga/desliga o led

  delay(1);
}



// TECLADO =======================================================================

unsigned char ler_teclado(){
  unsigned char j, tecla=0xFF, linha;
  clr_bit(COLUNA,coluna); //apaga o bit da coluna (varredura)

  linha = LINHA >> 4; //lê o valor das linhas

    for(j=0;j<4;j++) //testa as linhas
  	{
    	if(!tst_bit(linha,j))//se foi pressionada alguma tecla,
    	{ //decodifica e retorna o valor
      		tecla = pgm_read_byte(&teclado[j][coluna]);
      	}
    }
  set_bit(COLUNA,coluna); //ativa o bit zerado anteriormente
  return tecla; //retorna o valor 0xFF se nenhuma tecla foi pressionada
}

// verifica se uma tecla foi apertqa e adiciona ela a um vetor de senha
void press_key() {
  unsigned char nr;
  nr = ler_teclado(); //lê constantemente o teclado e recebe a tecla pressionada

  // debounce
  if (last_change_state[coluna] != nr && millis()-last_change_time>DEBOUNCE){
    last_change_time = millis(); // atualiza ultimo clique

    if (nr!=0xFF){ //se alguma tecla foi pressionada mostra seu valor
      unsigned char val = pgm_read_byte(&Tabela[nr<='9'?nr-'0':10+nr-'A']); // recebe código da tecla pressionada
      // write_disp(val); // escreve na tela

      // Serial.println(val); // verifica se o debounce esta correto
      // locked = !locked; // verifica debounce com o led PQ A PORRA DO PRINTLN NAO FUNCIONA ;_;
      // delay(1000);

      idx %= 4;
      if (idx<4) {
        password_attempt[idx+1] = password_attempt[idx]; // avança o digito
      	password_attempt[idx] = val; // adiciona tecla pressionada na senha
      }

      idx++;
    }

    last_change_state[coluna] = nr;
  }
}



// COLUNA =======================================================================

void column_change() {
  if (millis()>(last_column_change+1)) {
    mult_disp(coluna);

    coluna+=1;
    coluna%=4;
    last_column_change = millis();
  }
}



// TELA =======================================================================

void write_disp(unsigned char val){
  PORTC = val;
  PORTB &= ~(1<<PB0);
  PORTB |= (0x01)&(val>>6);
}

// chaveameto multiplexado aproveitando a coluna
void mult_disp(char coluna) {
  write_disp(0xFF);
  if (coluna == 0) {
    PORTB &= ~(1<<DIG4); // desliga o ultimo digito

    // escreve numero na tela
    if (idx > 0 && !(idx == 4)) {
      write_disp(password_attempt[idx-1]);
    }
    else {
      write_disp(0xFF); // limpa a tela
    }

    PORTB |= (1<<DIG1); // liga o ultimo digito
  }

  else if (coluna == 1) {
    PORTB &= ~(1<<DIG1);

    if (idx > 1 && !(idx == 4)) {
      write_disp(password_attempt[idx-2]);
    }
    else {
      write_disp(0xFF);
    }

    PORTB |= (1<<DIG2);
  }

  else if (coluna == 2) {
    PORTB &= ~(1<<DIG2);

    if (idx > 2 && !(idx == 4)) {
      write_disp(password_attempt[idx-3]);
    }
    else {
      write_disp(0xFF);
    }

    PORTB |= (1<<DIG3);
  }

  else if (coluna == 3) {
    PORTB &= ~(1<<DIG3);

    if (idx > 3) { // quando o idx chegar a 4 zera a tela
      write_disp(0xFF);
      PORTB &= ~(1<<DIG4);
    }

    // PORTB |= (1<<DIG4);
  }
}



// LED =======================================================================

// liga o led se a senha estiver certa e apaga se estiver errada
void display_led() {
  if (locked) {
    PORTB &= ~(1 << LED); // desliga led
  }
  else {
    PORTB |= (1 << LED); // liga led
  }
}



// OPERACOES COM SENHA =======================================================================

// verifica se a senha esta correta e bloqueia/libera acesso ao cofre (idicado pelo led)
void check_password() {
  bool flag = true;

  for (int i=0; i<4; i++) {
    if(password[i] != password_attempt[i]) {
      flag = false; // se algum número for diferente as senhas são diferentes
    }
  }

  if (flag) {
    locked = false; // libera acesso com senhas iguais
  }
  else {
    locked = true; // bloqueia acesso com senhas diferentes
  }
}

// salva a senha do cofre
void set_password() {

  // repete o loop principal porem apenas uma vez para definir a senha do cofre
  while (idx <= 4) {

    press_key();

    if (idx == 4) {
      idx++; // termina definição da senha
    }

    // anda pelas colunas
    column_change();

    display_led();

    delay(1);
  }

  // copia a senha
  for (int i=0; i<4; i++) {
    password[i] = password_attempt[i];
  }

  idx = 0; // reseta o idx
  locked = true; // trava o cofre
}
