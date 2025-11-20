                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             /****************************************Copyright (c)*************************************************
**                      Power Vending - A. R. M. Venda Automatica Ltda.
**                             http://www.powervending.com.br
**--------------Informação do Arquivo -----------------------------------------------------------------
** Nome do Arquivo:          firmwareMaqMoedas.ino
** Data Ultima Modificação:  14-12-17
** Ultima Versão:            Sim
** Descrição:                Software da controladora maquina de moedas.   
**------------------------------------------------------------------------------------------------------
** Criado por:          Marlon Zanardi <dev@powervending.com.br>
** Data de Criação:     18-10-17       
********************************************************************************************************/

/**********************************************************************************
** Observações:
** - Compilar utilizando versão com suporte a Serial HW com 9bits.
** - Alterar o tamanho do buffer RX de 64 para 128 bytes
**   <Arduino IDE>\hardware\arduino\avr\cores\arduino\HardwareSerial.h
**   "#define SERIAL_RX_BUFFER_SIZE 64" para "#define SERIAL_RX_BUFFER_SIZE 128"
***********************************************************************************/

// Bibliotecas
#include <Keypad.h>                      
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include "RTClib.h"
#include <Wire.h>
#include <avr/wdt.h>
#include "MDB.h"
#include "SensorQuedaInfra.h"
#include "Teclado.h"

/* Definição de versão do software e hardware. */
#define V_SFT 118
//Rele do MDB
#define RELE_MDB 23
//Constante para Ativo
#define ATIVO 1             
//Constante para Inativo
#define INATIVO 0  
//LED
#define RELE_1 A1
//MOTOR
#define RELE_2 A15
#define E_MDB 23
//PINO LDR
#define LDR A0
//Pinos do Display LCD.
#define LCD_RS 35
#define LCD_E 33
#define LCD_D4 31
#define LCD_D5 29
#define LCD_D6 27
#define LCD_D7 25
//Pinos teclado.
#define C1 49
#define C2 47
#define C3 45
#define C4 43
#define R1 41
#define R2 39
#define R3 37
//Linhas keypad
#define ROWS  3   
//Colunas keypad
#define COLS  4  
//Pino BUZZER
#define BUZZER 12
//Delay do buzzer
#define DELAY_BUZZER 80

// Endereços da EEPROM
#define EEPROM_ADDR_DEZ_EVENTOS_1 800
#define EEPROM_ADDR_DEZ_EVENTOS_2 801
#define EEPROM_ADDR_STATUS_VMC 999
#define EEPROM_ADDR_ESTOQUE_1 1001
#define EEPROM_ADDR_ESTOQUE_2 1002
#define EEPROM_ADDR_VALOR_TOTAL_1 1003
#define EEPROM_ADDR_VALOR_TOTAL_2 1004
#define EEPROM_ADDR_I_VALOR_TOTAL_1 1005
#define EEPROM_ADDR_I_VALOR_TOTAL_2 1006
#define EEPROM_ADDR_RECEITA_TOTAL_1 1007
#define EEPROM_ADDR_RECEITA_TOTAL_2 1008
#define EEPROM_ADDR_I_RECEITA_TOTAL_1 1009
#define EEPROM_ADDR_I_RECEITA_TOTAL_2 1010
#define EEPROM_ADDR_QTD_EVENTOS_FALHA_1 1011
#define EEPROM_ADDR_QTD_EVENTOS_FALHA_2 1012
#define EEPROM_ADDR_FIRST_TIME 2000
#define EEPROM_ADDR_TIPO_MAQUINA 2100
#define FIRST_TIME_MAGIC_VALUE 10

//Variaveis do DS1307
uint16_t startAddr = 0x0000;           
uint16_t lastAddr;                    
uint16_t TimeIsSet = 0xaa55; 

// Estrutura de eventos de falha.
struct event_falha
{
  int moedas_disp;
  DateTime data;
  long valor_depositado;
};

// Instancia da estrutura de eventos de falha.
event_falha info_falha;

// ============================================================================
// VARIÁVEIS GLOBAIS (97 total - REQUER REFATORAÇÃO FUTURA)
// ⚠️ ATENÇÃO: Excesso de variáveis globais dificulta manutenção!
// Ver REFACTORING_GUIDE.md para plano de migração para structs
// ============================================================================

// --- CONFIGURAÇÃO DO SISTEMA ---
int tipo_maquina = 0;              // Tipo/modelo da máquina (0=padrão)
int first_time = 0;                // Flag primeira execução (init EEPROM)
int status_maquina = 1;            // Status geral: 1=ativo, 0=inativo
int em_inicializacao = 0;          // Flag: sistema está inicializando

// --- CONTROLE DE ESTADO GERAL ---
int controle = 0;                  // Controle geral de estado (⚠️ uso ambíguo!)
int limpa_registro = 0;            // Flag para limpar registros
long time_start_infra = millis(), tempo_atual_infra = millis();
// Temporizador responsavel por fazer os dois pontos da hora piscarem no menu principal.
unsigned long tempo_atual_piscap = 0, time_start_piscap = 0;
// Armazenamento de data/hora
unsigned short int hora_1,hora_2;
unsigned short int minuto_1;
unsigned short int minuto_2;
unsigned short int dia_1;
unsigned short int dia_2;
unsigned short int mes_1; 
unsigned short int mes_2;
unsigned short int ano_1;
unsigned short int ano_2; 
int first_time = 0;
long pin_parte[6] = {0};
int ultimo_valor_inserido = 0;
int qtd_eventos_falha = 0;
DateTime data_evento_falha;
int controle_dez_eventos = 0;
int cont_segundos = 0;
int em_inicializacao = 0;
int aux_in = 0;
int altera_estado_notas = -1;
bool status_compra = 0;
int posicao_horario = 0;
bool teste_entrega = 0;
int divide_int = 0;
long estoque = 0;
long valor_real = 0;
unsigned int config_preco_valor[10] = {0};
char customKey = 0; 
// --- ESTADO DA VENDA (VMC - Vending Machine Controller) ---
int controle_vmc = 0;              // Estado da máquina de estados de venda
int valor_inserido = 0;            // Valor inserido pelo usuário (centavos)
bool em_venda = 0;                 // Flag: venda em andamento
bool controle_em_venda = 0;        // Controle auxiliar de venda
bool status_compra = 0;            // Status da compra atual
int posicao = 0;                   // Posição/seleção do produto
int aux = 0;                       // Auxiliar geral (⚠️ uso ambíguo!)

// --- HARDWARE: MOTOR E DISPENSADOR ---
long timeout_motor = 0;            // Timeout do motor (ms)
int controle_timeout_motor = 0;    // Controle do timeout do motor
int qtd_moedas_dispensar = 0;      // Quantidade de moedas a dispensar (troco)
int contador_moedas = 0;           // Contador de moedas dispensadas

// --- HARDWARE: SENSOR LDR E LEITURAS ---
int controle_ldr = 0;              // Controle do sensor LDR
int ldr_max = 0;                   // Valor máximo lido pelo LDR
int leitura_rep = 0;               // Repetições de leitura

// --- INTERFACE: DISPLAY E BUZZER ---
int controle_visualiza = 0;        // Controle de visualização
int mostra_msg_ini = 0;            // Flag: mostrar mensagem inicial
int linha_ini = 11;                // Linha inicial no display
int controle_buzzer = 0;           // Controle do buzzer
short controle_buzzer1 = 0;        // Controle auxiliar do buzzer

// --- TEMPORÁRIOS E AUXILIARES ---
unsigned int parte_1 = 0;          // Parte 1 para cálculos EEPROM
unsigned int parte_2 = 0;          // Parte 2 para cálculos EEPROM
// --- VENDAS E CONTABILIDADE (⚠️ CRÍTICO - Dados financeiros!) ---
long valor_total_inserido;         // EEPROM: Total inserido (histórico)
long i_valor_total_inserido;       // EEPROM: Parte inteira do total
long receita_total;                // EEPROM: Receita total arrecadada
long i_receita_total;              // EEPROM: Parte inteira da receita
long estoque = 0;                  // EEPROM: Quantidade em estoque
int ultimo_valor_inserido = 0;     // Último valor inserido (para display)
int qtd_eventos_falha = 0;         // EEPROM: Contador de falhas
int controle_dez_eventos = 0;      // EEPROM: Controle dos últimos 10 eventos

// --- MDB (Multi-Drop Bus - Comunicação com Moedeiro/Noteiro) ---
// ⚠️ CRÍTICO: Erros aqui podem causar perda de dinheiro!
bool boot_mdb = 0;                 // Flag: MDB completou boot
bool inicializacao_ok = 0;         // Flag: Inicialização MDB OK
bool status_vmc;                   // Status do VMC (vending machine)
int mdb_task_ctl = 0;              // Controle da task MDB
int controle_deposito_mdb = 0;     // Controle de depósito MDB

// --- MDB: NOTAS (BILL) ---
int controle_bill = 0;             // Estado da máquina de estados (bill)
int contador_bill = 0;             // Contador para bill
int lendo_bill = 0;                // Flag: lendo nota
bool mdb_bill_sem_atividade = 0;  // Flag: bill sem resposta
bool aguarda_reset_bill = 0;       // Flag: aguardando reset do bill
bool controle_ack = 0;             // Controle de ACK
bool temp_bill_teste = 0;          // Teste temporário de bill

// --- MDB: ESCROW (Custódia de Notas) ---
int escrow_ativo = 0;              // Flag: escrow ativo
int valor_inserido_bill_escrow = 0;// Valor da nota em escrow
int valor_inserido_bill = 0;       // Valor total inserido em notas
int type_escrow_1 = 0x00;          // Tipo escrow byte 1
int type_escrow_2 = 0x00;          // Tipo escrow byte 2
int bill_type_deposited[5];        // Tipo de nota depositada
int bill_routing[3];               // Roteamento da nota

// --- MDB: BUFFERS E DADOS ---
int data[100];                     // Buffer de dados MDB (100 bytes)
int dado_poll[10] = {0,0,0,0,0,0,0,0,0,0}; // Dados do poll MDB
int valor_inserido_total = 0;      // Valor total inserido (moedas+notas)

// --- VARIÁVEIS TEMPORÁRIAS E ÍNDICES ---
int contador = 0;                  // Contador geral
int i, msg;                        // Índices e mensagens temporárias
int i_mdb = 0;                     // Índice MDB
unsigned long int mult = 10000;    // Multiplicador para cálculos
// MILLIS
unsigned long tempo_atual_mdb=0 , time_start_mdb=0;
unsigned long tempo_atual_ldr=0 , time_start_ldr=0;
unsigned long tempo_atual_timeout_motor=0 , time_start_timeout_motor=0;
unsigned long tempo_atual=0 , time_start=0;
unsigned long tempo_atual_ini=0 , time_start_ini=0;
unsigned long tempo_atual_value=0 , time_start_value=0;
unsigned long tempo_atual_buzzer=0 , time_start_buzzer=0;
unsigned long tempo_atual_retorna=0 , time_start_retorna=0;
unsigned long tempo_atual_msg=0 , time_start_msg=0;
unsigned long tempo_atual_lcd=0 , time_start_lcd=0;
unsigned long tempo_atual_pisca=0 , time_start_pisca=0;
unsigned long tempo_atual_temp_bill=0 , time_start_temp_bill=0;
unsigned long tempo_atual_ack=0 , time_start_ack=0;
unsigned long tempo_atual_em_venda=0 , time_start_em_venda=0;
unsigned long tempo_atual_teste_entrega=0 , time_start_teste_entrega=0;

// Definicoes do display 20x4
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);  

// Definicoes do display 20x4
LiquidCrystal lcd2(22, 24, 26, 28, 30, 32);  

// Desenho Voltar do display
byte voltar[8] = {                                    
	B00001,
	B00001,
	B00001,
	B00101,
	B01001,
	B11111,
	B01000,
	B00100
};
// Desenho seta do display
byte seta[8] = {
	B00000,
	B00100,
	B01000,
	B11111,
	B01000,
	B00100,
	B00000,
	B00000
};
// Desenho moeda
byte moeda[8] = {
	B00000,
	B01110,
	B11111,
	B11111,
	B11111,
	B11111,
	B01110,
	B00000
};
// Tio
byte tio[8] = {
	B01101,
	B10010,
	B00000,
	B01110,
	B10001,
	B11111,
	B10001,
	B00000
};
// Cecedilia
byte cecedilia[8] = {
	B01110,
	B10001,
	B10000,
	B10000,
	B10001,
	B01110,
	B00010,
	B00110
};
// Acento no u.
byte uacento[8] = {
	B00010,
	B00100,
	B10001,
	B10001,
	B10001,
	B10001,
	B01110,
	B00000
};

// Intancia do rtc.
RTC_DS1307 rtc;
// Inicializacao das estruturas com valor 0.
INFO_BILL info_bill = 
{0,0,0,0,0,0,0,0,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0},{0,0}};  
// Instancia o mdb.
MDB mdb(RELE_MDB);
// Instancia o infra.
SensorQuedaInfra sensor_queda_infra;
Teclado teclado;

// Setup principal.
void setup() 
{
  //Inicializacao da serial 9600
  Serial.begin(115200); 
  //Inicializacao 9 bits de comunicacao do mdb
  Serial1.begin(9600, SERIAL_9N1); 
  // Inicialização da porta de comunicação Infra.
  Serial3.begin(9600);  
  // Inicializa os pinos.
  inicia_pinos();   
  // Inicializa a memoria e parametros iniciais.
  inicializacao();
  // Reset mdb(Inicializa).
  mdb.reset();
  
}

// Configura o relogio.
void setup_relogio()
{
  // Inicia o rtc.
  rtc.begin();  
  
  if (!rtc.isrunning()) {
    Serial.println("RTC parado, vou ajustar com a hora da compilacao...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  // se o byte 44 da memoria permanente do modulo contiver 
  // o valor 65 (colocado la anteriormente por este mesmo programa), 
  // sabemos que podemos exibir o dia e hora do ultimo reset.
  if (rtc.readnvram(44)==65) {
    Serial.print("O ultimo reset deste sistema foi no dia ");
    Serial.print(rtc.readnvram(40));
    Serial.print(" as ");
    Serial.print(rtc.readnvram(41));
    Serial.print(":");
    Serial.print(rtc.readnvram(42));
    Serial.print(":");
    Serial.print(rtc.readnvram(43));
    Serial.println(".");
  }  

  // grava dia/hora desta inicializacao na memoria permanente do Tiny RTC
  DateTime tstamp = rtc.now();
  rtc.writenvram(40, tstamp.day());
  rtc.writenvram(41, tstamp.hour());
  rtc.writenvram(42, tstamp.minute());
  rtc.writenvram(43, tstamp.second());
  rtc.writenvram(44, 65);
}

// Funcao que salva os dados na EEPROM.
void escreve_eeprom(int end_1, int end_2, int valor)
{
  EEPROM.write(end_1, valor/256);   
  EEPROM.write(end_2, valor%256); 
}

// Funcao que faz a leitura dos dados armazenados na EEPROM.
int read_eeprom(int ed_1, int ed_2)
{
  int valor;
  
  parte_1 = EEPROM.read(ed_1);
  parte_2 = EEPROM.read(ed_2); 
  valor = (parte_1*256) + parte_2;
  return valor;
}

// Realiza a inicializacao da memoria e de parametros do sistema.
void inicializacao()
{
  lcd.begin(20, 4); 
  lcd.createChar(3,voltar);         
  lcd.createChar(4,seta);
  lcd.createChar(5,moeda);
  lcd.createChar(6,tio);
  lcd.createChar(7,cecedilia);
  lcd.createChar(8,uacento);
  
  lcd2.begin(20, 4); 
  lcd2.createChar(3,voltar);         
  lcd2.createChar(4,seta);
  lcd2.createChar(5,moeda);
  lcd2.createChar(6,tio);
  lcd2.createChar(7,cecedilia);
  lcd2.createChar(8,uacento);
  
  mostra_inicializacao();
  
  time_start_retorna = millis(); 
  time_start_buzzer = millis();
  time_start_lcd = millis();
  time_start_piscap = millis();  
  
  status_vmc = EEPROM.read(EEPROM_ADDR_STATUS_VMC);
  aux = status_vmc;
  
  estoque = read_eeprom(EEPROM_ADDR_ESTOQUE_1, EEPROM_ADDR_ESTOQUE_2); 
  
  valor_total_inserido = read_eeprom(EEPROM_ADDR_VALOR_TOTAL_1, EEPROM_ADDR_VALOR_TOTAL_2);
  i_valor_total_inserido = read_eeprom(EEPROM_ADDR_I_VALOR_TOTAL_1, EEPROM_ADDR_I_VALOR_TOTAL_2);
  receita_total = read_eeprom(EEPROM_ADDR_RECEITA_TOTAL_1, EEPROM_ADDR_RECEITA_TOTAL_2);
  i_receita_total = read_eeprom(EEPROM_ADDR_I_RECEITA_TOTAL_1, EEPROM_ADDR_I_RECEITA_TOTAL_2);
  
  controle_dez_eventos = read_eeprom(EEPROM_ADDR_DEZ_EVENTOS_1, EEPROM_ADDR_DEZ_EVENTOS_2);  
  
  qtd_eventos_falha = read_eeprom(EEPROM_ADDR_QTD_EVENTOS_FALHA_1, EEPROM_ADDR_QTD_EVENTOS_FALHA_2);  
  
  //Inicializacao do protocolo DS1307
  setup_relogio();  
  
  first_time = EEPROM.read(EEPROM_ADDR_FIRST_TIME);  
  
  tipo_maquina = EEPROM.read(EEPROM_ADDR_TIPO_MAQUINA); 
  
  // Verifica se é a primeira vez que o codigo foi compilado.
  if ( first_time != FIRST_TIME_MAGIC_VALUE )
  {
    controle_dez_eventos = 0; 
    escreve_eeprom(EEPROM_ADDR_DEZ_EVENTOS_1, EEPROM_ADDR_DEZ_EVENTOS_2, controle_dez_eventos);
    
    qtd_eventos_falha = 0; 
    escreve_eeprom(EEPROM_ADDR_QTD_EVENTOS_FALHA_1, EEPROM_ADDR_QTD_EVENTOS_FALHA_2, qtd_eventos_falha);
    
    status_vmc = 0;
    aux = status_vmc;
    EEPROM.write(EEPROM_ADDR_STATUS_VMC, status_vmc);

    estoque = 0;
    escreve_eeprom(EEPROM_ADDR_ESTOQUE_1, EEPROM_ADDR_ESTOQUE_2, estoque);
    
    valor_total_inserido = 0;
    i_valor_total_inserido = 0;
    receita_total = 0;
    i_receita_total = 0;
    
    escreve_eeprom(EEPROM_ADDR_VALOR_TOTAL_1, EEPROM_ADDR_VALOR_TOTAL_2, valor_total_inserido);
    escreve_eeprom(EEPROM_ADDR_I_VALOR_TOTAL_1, EEPROM_ADDR_I_VALOR_TOTAL_2, i_valor_total_inserido);
    escreve_eeprom(EEPROM_ADDR_RECEITA_TOTAL_1, EEPROM_ADDR_RECEITA_TOTAL_2, receita_total);
    escreve_eeprom(EEPROM_ADDR_I_RECEITA_TOTAL_1, EEPROM_ADDR_I_RECEITA_TOTAL_2, i_receita_total);
    
    tipo_maquina = 0;
    EEPROM.write(EEPROM_ADDR_TIPO_MAQUINA, tipo_maquina);  
    
    EEPROM.write(EEPROM_ADDR_FIRST_TIME, FIRST_TIME_MAGIC_VALUE);    
     
  }
}

// Inicia os pinos da controladora.
void inicia_pinos()
{
  //LDR
  analogReference(EXTERNAL);

  //LED
  pinMode(RELE_1, OUTPUT);
  digitalWrite(RELE_1,LOW);
  //MDB
  pinMode(RELE_MDB, OUTPUT);
  digitalWrite(RELE_MDB,LOW);
  //MOTOR
  pinMode(RELE_2, OUTPUT);  
  digitalWrite(RELE_2,LOW);  
  //Pino Buzzer
  pinMode(BUZZER,OUTPUT);
  digitalWrite(BUZZER,LOW);
  
  pinMode(LDR,INPUT_PULLUP);
  
  pinMode(13,OUTPUT);
  digitalWrite(13,HIGH);
}

// Ao Iniciar sistema, tela de "carregamento".
void mostra_inicializacao()
{
  lcd.clear();
  lcd.setCursor(3,0);
  lcd.print(F("INICIALIZANDO"));
  lcd.setCursor(6,1);
  lcd.print(F("SISTEMA")); 
  lcd.setCursor(4,2);
  lcd.print(F("AGUARDE"));                      	
  lcd.setCursor(0,3);
  lcd.print(F("POWER VENDING V")); 
  lcd.setCursor(15,3);
  lcd.print((V_SFT));
  lcd.setCursor(17,3);
  lcd.print(F("."));   
  
  lcd2.clear();
  lcd2.setCursor(3,0);
  lcd2.print(F("INICIALIZANDO"));
  lcd2.setCursor(6,1);
  lcd2.print(F("SISTEMA")); 
  lcd2.setCursor(4,2);
  lcd2.print(F("AGUARDE"));                      	
  lcd2.setCursor(0,3);
  lcd2.print(F("POWER VENDING V")); 
  lcd2.setCursor(15,3);
  lcd2.print((V_SFT));
  lcd2.setCursor(17,3);
  lcd2.print(F("."));   
}

// Funcao para contar o tempo da inicializacao do sistema e imprimir a sequencias de pontos(. . .)
void aguarda_inicializacao()
{
  if(mostra_msg_ini==0)
  {
    mostra_inicializacao();
    mostra_msg_ini=1;
  }
  if(inicializacao_ok==0)
  {     
    tempo_atual= millis();
    //A cada 1s imprime um ponto.
    if((tempo_atual-time_start) > 1000)
      {
          time_start = tempo_atual; 
          lcd.setCursor(linha_ini,2);
          lcd.print(F("."));
          lcd2.setCursor(linha_ini,2);
          lcd2.print(F("."));
          linha_ini++;
      }
  }
  tempo_atual_ini = millis();
  //Quando chegar a 5 segundos, finaliza a inicializacao.
  if((tempo_atual_ini-time_start_ini) > 6000)
  {
    inicializacao_ok = 1;
    
    em_inicializacao = 1;
  }  
  
  if ( em_inicializacao == 1 && aux_in == 0)
  {
    lcd2.clear();
    lcd2.setCursor(0,1);
    lcd2.print(F("     EM SELECAO"));
    lcd2.setCursor(0,2);
    lcd2.print(F("      DE VENDA"));
    
    aux_in = 1;
  }
}

// Reinicia a tela de lcd.
void reinicia_lcd()
{
  if(inicializacao_ok)
  {
    lcd.begin(20,4);
    lcd.createChar(3,voltar);         
    lcd.createChar(4,seta);
    lcd.createChar(5,moeda);
    lcd.createChar(6,tio);
    lcd.createChar(7,cecedilia);
    lcd.createChar(8,uacento);
  }
}

// Menu 04, configuracao da hora e data. 
void menu_horario()
{
  switch(posicao_horario)
  {
    case 0:
          lcd2.clear();
          lcd2.setCursor(0,0);
          lcd2.print(F("MENU 04.1"));
          lcd2.setCursor(0,1);
          lcd2.print(F("CONFIG. HORA"));	
          lcd2.setCursor(0,3);
          lcd2.print(F("CONFIRMA? OK /  "));
          lcd2.setCursor(15,3);
          lcd2.write(3);
          break;
     case 1:
          lcd2.clear();
          lcd2.setCursor(0,0);
          lcd2.print(F("MENU 04.2"));
          lcd2.setCursor(0,1);
          lcd2.print(F("CONFIG. DATA"));	
          lcd2.setCursor(0,3);
          lcd2.print(F("CONFIRMA? OK /  "));
          lcd2.setCursor(15,3);
          lcd2.write(3);
          break;
  }
}

// Menu de relatorios.
void menu_relatorio()
{
  switch(posicao)
  {
    case 0:
        lcd2.clear();
        lcd2.setCursor(0,0);
        lcd2.print(F("MENU 01.1"));
        lcd2.setCursor(0,1);
        lcd2.print(F("RELATORIO ATIVO"));	
        lcd2.setCursor(0,3);
        lcd2.print(F("CONFIRMA? OK /  "));
        lcd2.setCursor(15,3);
        lcd2.write(3);        
        break;  
    case 1:
        lcd2.clear();
        lcd2.setCursor(0,0);
        lcd2.print(F("MENU 01.2"));
        lcd2.setCursor(0,1);
        lcd2.print(F("RELATORIO"));	
        lcd2.setCursor(0,2);
        lcd2.print(F("PERMANENTE"));
        lcd2.setCursor(0,3);
        lcd2.print(F("CONFIRMA? OK /  "));
        lcd2.setCursor(15,3);
        lcd2.write(3);        
        break;    
    case 2:
        lcd2.clear();
        lcd2.setCursor(0,0);
        lcd2.print(F("MENU 01.3"));
        lcd2.setCursor(0,1);
        lcd2.print(F("RESETA RELATORIOS"));	
        lcd2.setCursor(0,3);
        lcd2.print(F("CONFIRMA? OK /  "));
        lcd2.setCursor(15,3);
        lcd2.write(3);      
        break; 
     case 3:
        lcd2.clear();
        lcd2.setCursor(0,0);
        lcd2.print(F("MENU 01.4"));
        lcd2.setCursor(0,1);
        lcd2.print(F("EVENTOS DE FALHA"));	
        lcd2.setCursor(0,3);
        lcd2.print(F("CONFIRMA? OK /  "));
        lcd2.setCursor(15,3);
        lcd2.write(3);      
        break;    
  }
}

// Menu de testes.
void menu_testes()
{
  switch(posicao)
  {
    case 0:
        lcd2.clear();
        lcd2.setCursor(0,0);
        lcd2.print(F("MENU 03.1"));
        lcd2.setCursor(0,1);
        lcd2.print(F("TESTE DE ENTREGA"));	
        lcd2.setCursor(0,3);
        lcd2.print(F("CONFIRMA? OK /  "));
        lcd2.setCursor(15,3);
        lcd2.write(3);        
        break;  
    case 1:
        lcd2.clear();
        lcd2.setCursor(0,0);
        lcd2.print(F("MENU 03.2"));
        lcd2.setCursor(0,1);
        lcd2.print(F("DISPENSA MANUAL"));
        lcd2.setCursor(0,3);
        lcd2.print(F("CONFIRMA? OK /  "));
        lcd2.setCursor(15,3);
        lcd2.write(3);        
        break;    
    case 2:
        lcd2.clear();
        lcd2.setCursor(0,0);
        lcd2.print(F("MENU 03.3"));
        lcd2.setCursor(0,1);
        lcd2.print(F("LDR"));	
        lcd2.setCursor(0,3);
        lcd2.print(F("CONFIRMA? OK /  "));
        lcd2.setCursor(15,3);
        lcd2.write(3);      
        break;    
  }
}

// Menu secundario, menu principal de servico, onde contem todos menus de acesso.
void menu_servico()
{
  switch(posicao)
  {
    case 0:
        lcd2.clear();
        lcd2.setCursor(0,0);
        lcd2.print(F("MENU 01"));
        lcd2.setCursor(0,1);
        lcd2.print(F("RELATORIO DE VENDAS"));	
        lcd2.setCursor(0,3);
        lcd2.print(F("CONFIRMA? OK /  "));
        lcd2.setCursor(15,3);
        lcd2.write(3);        
        break;  
    case 1:
        lcd2.clear();
        lcd2.setCursor(0,0);
        lcd2.print(F("MENU 02"));
        lcd2.setCursor(0,1);
        lcd2.print(F("CONFIG ESTOQUE ("));
        lcd2.setCursor(16,1);
        if(estoque>99)
        {
          lcd2.print(estoque); 
        }
        if(estoque>9 && estoque<100)
        {
          lcd2.print(F("0"));
          lcd2.setCursor(17,1);
          lcd2.print(estoque); 
        }
        if(estoque<10)
        {
          lcd2.print(F("00"));
          lcd2.setCursor(18,1);
          lcd2.print(estoque); 
        }
        lcd2.setCursor(19,1);
        lcd2.print(F(")"));
        lcd2.setCursor(0,3);
        lcd2.print(F("CONFIRMA? OK /  "));
        lcd2.setCursor(15,3);
        lcd2.write(3);        
        break;    
    case 2:
        lcd2.clear();
        lcd2.setCursor(0,0);
        lcd2.print(F("MENU 03"));
        lcd2.setCursor(0,1);
        lcd2.print(F("TESTES"));	
        lcd2.setCursor(0,3);
        lcd2.print(F("CONFIRMA? OK /  "));
        lcd2.setCursor(15,3);
        lcd2.write(3);      
        break;
    case 3:
        lcd2.clear();
        lcd2.setCursor(0,0);
        lcd2.print(F("MENU 04"));
        lcd2.setCursor(0,1);
        lcd2.print(F("CONFIG RELOGIO"));	
        lcd2.setCursor(0,3);
        lcd2.print(F("CONFIRMA? OK /  "));
        lcd2.setCursor(15,3);
        lcd2.write(3);      
        break;     
    case 4:
        lcd2.clear();
        lcd2.setCursor(0,0);
        lcd2.print(F("MENU 05"));
        lcd2.setCursor(0,1);
        lcd2.print(F("RESET DO SISTEMA"));	
        lcd2.setCursor(0,3);
        lcd2.print(F("CONFIRMA? OK /  "));
        lcd2.setCursor(15,3);
        lcd2.write(3);      
        break;  
    case 5:
        lcd2.clear();
        lcd2.setCursor(0,0);
        lcd2.print(F("MENU 06"));
        lcd2.setCursor(0,1);
        lcd2.print(F("TIPO MAQUINA"));	
        lcd2.setCursor(0,3);
        lcd2.print(F("CONFIRMA? OK /  "));
        lcd2.setCursor(15,3);
        lcd2.write(3);      
        break;     
  }
}

// Máquina de estado para controladora(tela inicial e serviço).
void statemachine_vmc()
{
  // Se inicializacao ja realizada.
  if ( inicializacao_ok )
  {
    switch(controle_vmc)
    {
      case 0:
            // Se maquina operando.
            if ( status_vmc )
            {
              // Se estoque maior que quatro.
              if ( estoque > 4 )
              {
                lcd.clear();
                lcd.setCursor(0,0);
                lcd.print(F(" LEVE UMA LEMBRANCA"));
                // Se maquina de aparecida.
                if ( !tipo_maquina )
                {
                  lcd.setCursor(0,1);
                  lcd.print(F("DO SANT DE APARECIDA")); 
                // Se maquina do rio de janeiro. 
                }else
                {
                  lcd.setCursor(0,1);
                  lcd.print(F("  DO PAO DE ACUCAR")); 
                  lcd.setCursor(6,1);
                  lcd.write(6);
                  lcd.setCursor(13,1);
                  lcd.write(7);
                  lcd.setCursor(14,1);
                  lcd.write(8);
                }
                lcd.setCursor(0,3);
                lcd.print(F("   MOEDAS  BRASIL"));
                lcd.setCursor(2,3);
                lcd.write(0b00100010);  
                lcd.setCursor(17,3);
                lcd.write(0b00100010);
                printDate();
              // Mostra equipamento sem estoque.  
              }else{
                      Serial.println("ENTROU EQUIPAMENTO SEM ESTOQUE");
                      lcd.clear();
                      lcd.setCursor(0,1);
                      lcd.print(F("    EQUIPAMENTO"));
                      lcd.setCursor(0,2);
                      lcd.print(F("                  "));
                      lcd.setCursor(0,2);
                      lcd.print(F("    SEM ESTOQUE")); 
                    }
            // Mostra equipamento em manutencao.        
            }else{  
                    lcd.clear();
                    lcd.setCursor(0,1);
                    lcd.print(F("   EQUIPAMENTO EM"));
                    lcd.setCursor(0,2);
                    lcd.print(F("     MANUTENCAO."));
                  }
            controle_vmc = 20;
            break;
      // Aguardando alguma interacao com o teclado para entrar no menu de serviço.     
      case 20:   
             customKey = teclado.leitura(); 
             if ( customKey != NO_KEY )
             { 
                // Ativa controle do buzzer para emitir o beep.
                controle_buzzer = 1;
                time_start_buzzer = millis(); 
                time_start_retorna = millis(); 
                // Continua para proximo estado.
                controle_vmc++;
                teclado.valor_lido();
             }               
             break; 
      // Mostra menu de serviço e altera mensagem do display 1.       
      case 21:
            // Mostra menu de serviço.
            menu_servico();
            // Mostra mensagem na tela de interacao com o usuario, que a maquina esta em menu de serviço.
            lcd.clear();
            lcd.setCursor(0,1);
            lcd.print(F("     EM MENU DE"));
            lcd.setCursor(0,2);
            lcd.print(F("      SERVICO"));
            // Continua para proximo estado.
            controle_vmc++;
            break; 
      // Percorre o menu atravez do teclado matricial.        
      case 22:                  
             customKey = teclado.leitura(); 
             if ( customKey != NO_KEY )
             {
               // Ativa controle do buzzer para emitir o beep.
               controle_buzzer = 1;
               time_start_buzzer = millis();
               time_start_retorna = millis();
               switch(customKey)
               {
                 // Botao 8 para cima.
                 case '8':
                         // Percorre o menu.
                         if ( posicao < 5 )
                            posicao++;
                         else
                           posicao = 0; 
                          
                          controle_vmc = 21;
                          break;
                 // Botao 2 para baixo.         
                 case '2':
                          // Percorre o menu.
                          if ( posicao > 0 )                          
                            posicao--;
                          else
                           posicao = 5; 
                          
                          controle_vmc = 21;
                          break;
                 // Botao OK, entra no menu selecionado.         
                 case 'B':
                          switch(posicao)
                          {
                            // Relatorio de vendas.
                            case 0:                     
                                   controle_vmc = 45;
                                   break;
                            // Estoque.     
                            case 1:                     
                                   controle_vmc = 23;
                                   break;
                            // Testes.     
                            case 2:                    
                                  controle_vmc = 55;
                                  break;
                            // Reset do sistema.
                            case 3:                    
                                  controle_vmc = 63;
                                  break; 
                            // Congiguracao do equipamento.        
                            case 4:                    
                                  softReset();                                
                                  break;    
                            // Tipo de maquina.       
                            case 5:                    
                                  controle_vmc = 110;                                
                                  break;       
                          }        
                          posicao = 0;                     
                          break;
                 // Botao voltar, volta para o menu central.         
                 case 'A':         
                        controle_vmc = 0;
                        posicao = 0;
                        // Muda mensagem do display 2.
                        lcd2.clear();
                        lcd2.setCursor(0,1);
                        lcd2.print(F("     EM SELECAO"));
                        lcd2.setCursor(0,2);
                        lcd2.print(F("      DE VENDA"));
                        break;
               }
               teclado.valor_lido();
             }       
            break;
      // Selecao de tipo da maquina.      
      case 110:     
          // Mostra indicação de digito.
          lcd2.clear();        
          lcd2.setCursor(0,0);
          lcd2.print(F("   1   /   2"));
          lcd2.setCursor(0,1);
          lcd2.print(F("  RIO  / APARECIDA")); 
          lcd2.setCursor(0,2);
          lcd2.print(F("ATUAL:            "));
          // Verifica qual o tipo de maquina atual.
          if( tipo_maquina )
          {
            lcd2.setCursor(6,2);
            lcd2.print(F(" RIO   "));
          }else
          {
            lcd2.setCursor(6,2);
            lcd2.print(F(" APARECIDA    "));
          }
          lcd2.setCursor(0,3);
          lcd2.print(F("OK /  "));
          lcd2.setCursor(5,3);
          lcd2.write(3); 
          // Continua para proximo case.
          controle_vmc++;
          break;
   // Aguarda interacao com o teclado matricial.       
   case 111:
          customKey = teclado.leitura(); 
          if ( customKey != NO_KEY )
          {
            // Ativa controle do buzzer para emitir o beep.
            controle_buzzer = 1;
            time_start_buzzer = millis();
            time_start_retorna = millis();
             
            switch(customKey)
            {
              // Botao voltar, volta para menu.
              case 'A':                
                      controle_vmc = 21;
                      break;
              // Botao OK, volta para menu.   
              case 'B':                      
                      controle_vmc = 21;
                      break;
              // Botao 1, muda o tipo de maquina para rio. 
              case '1':
                      lcd2.setCursor(6,2);
                      lcd2.print(F(" RIO         "));
                      tipo_maquina = 1;
                      EEPROM.write(2100, tipo_maquina);
                      break;
              // Botao 2, muda o tipo de maquina para aparecida.      
              case '2':
                      lcd2.setCursor(6,2);
                      lcd2.print(F(" APARECIDA    "));
                      tipo_maquina = 0;
                      EEPROM.write(2100, tipo_maquina);
                      break; 
            }
            teclado.valor_lido();
          }            
          break;   
//---------------------------------ESTOQUE------------------------------
      case 23:
            lcd2.clear();
            lcd2.setCursor(0,0);
            lcd2.print(F("MENU 02.1"));
            lcd2.setCursor(0,1);
            lcd2.print(F("ADICIONAR ESTOQUE"));
            lcd2.setCursor(0,3);
            lcd2.print(F("CONFIRMA? OK /  "));
            lcd2.setCursor(15,3);
            lcd2.write(3); 
            controle_vmc++;    
            break;
      case 24:
            customKey = teclado.leitura(); 
            if ( customKey != NO_KEY )
            {
             // Ativa controle do buzzer para emitir o beep.
             controle_buzzer = 1;
             time_start_buzzer = millis();   
             if( customKey== 'A' || customKey == 'B' )
             {
                 if ( customKey == 'A' )
                 {
                    controle_vmc = 21;
                 }
                 if ( customKey == 'B' )
                 {
                    controle_vmc = 27;
                 }
               }else if( customKey == '2' || customKey == '8' )
               {
                 controle_vmc++;
               }
               teclado.valor_lido();
             }
             break;  
      case 25:  
            lcd2.clear();
            lcd2.setCursor(0,0);
            lcd2.print(F("MENU 02.2"));
            lcd2.setCursor(0,1);
            lcd2.print(F("ALTERAR ESTOQUE"));
            lcd2.setCursor(0,3);
            lcd2.print(F("CONFIRMA? OK /  "));
            lcd2.setCursor(15,3);
            lcd2.write(3);  
            controle_vmc++;  
            break;
      case 26:
            customKey = teclado.leitura(); 
            if ( customKey != NO_KEY )
            {
             //Ativa controle do buzzer para emitir o beep.
             controle_buzzer=1;
             time_start_buzzer=millis();   
             if(customKey== 'A' || customKey == 'B')
             {
                 if(customKey== 'A')
                 {
                    controle_vmc = 21;
                 }
                 if(customKey== 'B')
                 {
                    controle_vmc = 36;
                 }
               }else if( customKey == '2' || customKey == '8' )
               {
                 controle_vmc = 23;
               }
               teclado.valor_lido();
             }
             break; 
      case 27:
            lcd2.clear();  
            lcd2.setCursor(0,0);
            lcd2.print(F("MENU 02.1.1"));
            lcd2.setCursor(0,1);
            lcd2.print(F("DIGITE O VALOR:")); 
            lcd2.setCursor(0,2);
            lcd2.print(F("____"));
            lcd2.setCursor(5,2);
            lcd2.print(F("EST. ATUAL     "));
            lcd2.setCursor(16,2);
            lcd2.print(estoque);	
            lcd2.setCursor(0,3);         
            lcd2.print(F("VOLTAR  "));
            lcd2.setCursor(7,3);
            lcd2.write(3); 
            controle_vmc++;
            break; 
      //Aguarda o primeiro valor ser digitado no teclado matricial.           
    case 28:
          lcd2.blink();
          lcd2.setCursor(0,2);
          customKey = teclado.leitura(); 
          if ( customKey != NO_KEY )
          {
              // Ativa controle do buzzer para emitir o beep.
              controle_buzzer = 1;
              time_start_buzzer = millis();  
              if( customKey == 'A' || customKey == 'B' )
              {
                // Botao voltar, volta para selecionar a posicao novamente.
                if(customKey =='A')
                {
                  lcd2.noBlink();
                  controle_vmc = 23;
                  break;
                }
              } else { 
              // Armazena o valor e mostra na tela.  
              config_preco_valor[0] = customKey;                
              lcd2.print(customKey);
              lcd2.setCursor(0,3);
              lcd2.print(F("                   "));
              lcd2.setCursor(0,3);
              lcd2.print(F("CORRIGIR  "));
              lcd2.setCursor(10,3);
              lcd2.write(3);               
              controle_vmc++;  
              }      
              teclado.valor_lido();                     
          }                   
          break;
   // Aguarda o proximo valor ser digitado no teclado matricial.       
   case 29:   
           lcd2.setCursor(1,2);
           customKey = teclado.leitura(); 
           if ( customKey != NO_KEY )
           {
             // Ativa controle do buzzer para emitir o beep.
             controle_buzzer = 1;
             time_start_buzzer = millis();  
             if( customKey == 'A' || customKey == 'B' )
             {
               //Botao voltar, volta para aguardar o valor anterior.
               if(customKey =='A')
               {
                 lcd2.setCursor(0,3);
                 lcd2.print(F("                   "));
                 lcd2.setCursor(0,3);
                 lcd2.print(F("VOLTAR  "));
                 lcd2.setCursor(7,3);
                 lcd2.write(3);
                 lcd2.setCursor(0,2);
                 lcd2.print(F("_"));
                 controle_vmc=28;
               //Botao OK, automaticamente completa os campos, jogando os valores digitados para ultimas casas.  
               }else{
                       lcd2.noBlink();
                       lcd2.setCursor(0,2);
                       lcd2.print(F("000"));
                       lcd2.setCursor(3,2);
                       lcd2.write(config_preco_valor[0]);
                       config_preco_valor[3] = config_preco_valor[0];
                       for(int i=0; i<3;i++)
                       {
                         config_preco_valor[i] = 48;
                       }
                       controle_vmc=32;
                     }
             }  else {
               //Armazena o valor e mostra na tela.  
               config_preco_valor[1] = customKey;                
               lcd2.print(customKey);                           
               controle_vmc++; 
             }     
             teclado.valor_lido();                      
           }  
           break;
   // Aguarda o proximo valor ser digitado no teclado matricial.             
   case 30:
           lcd2.setCursor(2,2);
           customKey = teclado.leitura(); 
           if ( customKey != NO_KEY )
           {
             //Ativa controle do buzzer para emitir o beep.
             controle_buzzer = 1;
             time_start_buzzer = millis();  
             if(customKey== 'A' || customKey == 'B')
             {
               //Botao voltar, volta para aguardar o valor anterior.
               if(customKey =='A')
               {
                 lcd2.setCursor(1,2);
                 lcd2.print(F("_"));
                 controle_vmc=29;
               //Botao OK, automaticamente completa os campos, jogando os valores digitados para ultimas casas. 
               }else{
                       lcd2.noBlink();
                       lcd2.setCursor(0,2);
                       lcd2.print(F("00"));
                       lcd2.setCursor(2,2);
                       lcd2.write(config_preco_valor[0]);
                       lcd2.setCursor(3,2);
                       lcd2.write(config_preco_valor[1]);
                       config_preco_valor[2] = config_preco_valor[0];
                       config_preco_valor[3] = config_preco_valor[1];
                       for(int i=0; i<2;i++)
                       {
                         config_preco_valor[i] = 48;
                       }
                       controle_vmc=32;
                     }
             }  else {
               //Armazena o valor e mostra na tela.  
               config_preco_valor[2] = customKey;                
               lcd2.print(customKey);                           
               controle_vmc++; 
             } 
             teclado.valor_lido();                          
           } 
           break;
   // Aguarda o proximo valor ser digitado no teclado matricial.   
   case 31:
           lcd2.setCursor(3,2);
           customKey = teclado.leitura(); 
           if ( customKey != NO_KEY )
           {
              // Ativa controle do buzzer para emitir o beep.
              controle_buzzer = 1;
              time_start_buzzer = millis();  
             if( customKey== 'A' || customKey == 'B' )
             {
               //Botao voltar, volta para aguardar o valor anterior.
               if( customKey == 'A' )
               {
                 lcd2.setCursor(2,2);
                 lcd2.print(F("_"));
                 controle_vmc = 30;
               // Botao OK, automaticamente completa os campos, jogando os valores digitados para ultimas casas.    
               }else{
                     lcd2.noBlink();
                     lcd2.setCursor(0,2);
                     lcd2.print(F("0"));
                     lcd2.setCursor(1,2);
                     lcd2.write(config_preco_valor[0]);
                     lcd2.setCursor(2,2);
                     lcd2.write(config_preco_valor[1]);
                     lcd2.setCursor(3,2);
                     lcd2.write(config_preco_valor[2]);
                     config_preco_valor[3] = config_preco_valor[2];
                     config_preco_valor[2] = config_preco_valor[1];
                     config_preco_valor[1] = config_preco_valor[0]; 
                     config_preco_valor[0] = 48;                        
                     controle_vmc++;
               }
             }else{
               //Armazena o valor e mostra na tela.  
               config_preco_valor[3] = customKey;                
               lcd2.print(customKey); 
               lcd2.noBlink();               
               controle_vmc++; 
             }    
             teclado.valor_lido();                       
           } 
           break;    
   // Apos dados selecionados, mostra a confirmacao. 
   case 32:
          lcd2.setCursor(0,3);
          lcd2.print(F("CONFIRMA? OK /  "));
          lcd2.setCursor(15,3);
          lcd2.write(3);  
          controle_vmc++;  
          break;
   // Aguarda confirmacao ou voltar.              
   case 33:
          customKey = teclado.leitura(); 
          if ( customKey != NO_KEY )
          {
               // Ativa controle do buzzer para emitir o beep.
               controle_buzzer = 1;
               time_start_buzzer = millis();  
               if( customKey == 'A' || customKey == 'B' )
               {
                 // Botao voltar, volta para aguardar o ultimo valor digitado.
                 if ( customKey == 'A' )
                 {
                   lcd2.blink();
                   lcd2.setCursor(3,2);                      
                   lcd2.print(F("_"));
                   lcd2.setCursor(0,3);
                   lcd2.print(F("                   "));
                   lcd2.setCursor(0,3);
                   lcd2.print(F("CORRIGIR  "));
                   lcd2.setCursor(10,3);
                   lcd2.write(3);
                   controle_vmc=31;
                   break;
                 }
                 // Botao OK, prosseguir para armazenar o dado.
                 if ( customKey == 'B' )
                 {
                   controle_vmc++;
                 }
               }
               teclado.valor_lido();                                                             
              }                    
           break;  
   // Calculo para transformar o dados recebidos em um unico dado.        
   case 34:
          mult = 1000;
          valor_real = 0;
          // Logica para calcular os dados e transformá-los em um inteiro.
          for(int i=0; i<4;i++)
          {
            config_preco_valor[i] = (config_preco_valor[i] - 48)*mult;
            valor_real  = valor_real + config_preco_valor[i];            
            mult= mult/10;
          } 
          estoque = estoque + valor_real; 
          escreve_eeprom(1001,1002,estoque);
          lcd2.clear();  
          lcd2.setCursor(0,1);
          lcd2.print(F("ESTOQUE ATUALIZADO"));
          lcd2.setCursor(0,2);
          lcd2.print(F("COM SUCESSO!"));    
          time_start_msg = millis();
          controle_vmc++;
          break; 
      case 35:
          tempo_atual_msg = millis();
          if((tempo_atual_msg-time_start_msg) > 1500)
          {
            controle_vmc = 81;       
          } 
          break; 
//----------------------------ALTERACAO DE ESTOQUE-------------------------------
      case 36:
          lcd2.clear();  
          lcd2.setCursor(0,0);
          lcd2.print(F("MENU 02.2.1"));
          lcd2.setCursor(0,1);
          lcd2.print(F("DIGITE O VALOR:")); 
          lcd2.setCursor(0,2);
          lcd2.print(F("____"));
          lcd2.setCursor(5,2);
          lcd2.print(F("EST. ATUAL     "));
          lcd2.setCursor(16,2);
          lcd2.print(estoque);	
          lcd2.setCursor(0,3);         
          lcd2.print(F("VOLTAR  "));
          lcd2.setCursor(7,3);
          lcd2.write(3); 
          controle_vmc++;
          break; 
    // Aguarda o primeiro valor ser digitado no teclado matricial.           
    case 37:
          lcd2.blink();
          lcd2.setCursor(0,2);
          customKey = teclado.leitura(); 
          if ( customKey != NO_KEY )
          {
             //Ativa controle do buzzer para emitir o beep.
             controle_buzzer = 1;
             time_start_buzzer = millis();  
             if ( customKey== 'A' || customKey == 'B' )
             {
               //Botao voltar, volta para selecionar a posicao novamente.
               if(customKey =='A')
               {
                 lcd2.noBlink();
                 controle_vmc = 25;
                 break;
               }
             }else
             { 
               //Armazena o valor e mostra na tela.  
               config_preco_valor[0] = customKey;                
               lcd2.print(customKey);
               lcd2.setCursor(0,3);
               lcd2.print(F("                   "));
               lcd2.setCursor(0,3);
               lcd2.print(F("CORRIGIR  "));
               lcd2.setCursor(10,3);
               lcd2.write(3);               
               controle_vmc++;  
             } 
             teclado.valor_lido();                          
          }                   
          break;
   //Aguarda o proximo valor ser digitado no teclado matricial.       
   case 38:   
           lcd2.setCursor(1,2);
           customKey = teclado.leitura(); 
           if ( customKey != NO_KEY )
           {
             // Ativa controle do buzzer para emitir o beep.
             controle_buzzer = 1;
             time_start_buzzer = millis();  
             if ( customKey == 'A' || customKey == 'B' )
             {
               //Botao voltar, volta para aguardar o valor anterior.
               if ( customKey == 'A' )
               {
                 lcd2.setCursor(0,3);
                 lcd2.print(F("                   "));
                 lcd2.setCursor(0,3);
                 lcd2.print(F("VOLTAR  "));
                 lcd2.setCursor(7,3);
                 lcd2.write(3);
                 lcd2.setCursor(0,2);
                 lcd2.print(F("_"));
                 controle_vmc = 37;
               // Botao OK, automaticamente completa os campos, jogando os valores digitados para ultimas casas.  
               }else{
                       lcd2.noBlink();
                       lcd2.setCursor(0,2);
                       lcd2.print(F("000"));
                       lcd2.setCursor(3,2);
                       lcd2.write(config_preco_valor[0]);
                       config_preco_valor[3] = config_preco_valor[0];
                       for(int i=0; i<3;i++)
                       {
                         config_preco_valor[i] = 48;
                       }
                       controle_vmc = 41;
                     }
             }  else {
             //Armazena o valor e mostra na tela.  
             config_preco_valor[1] = customKey;                
             lcd2.print(customKey);                           
             controle_vmc++; 
             }     
             teclado.valor_lido();                      
          }  
          break;
   //Aguarda o proximo valor ser digitado no teclado matricial.             
   case 39:
           lcd2.setCursor(2,2);
           customKey = teclado.leitura(); 
           if ( customKey != NO_KEY )
           {
               // Ativa controle do buzzer para emitir o beep.
               controle_buzzer = 1;
               time_start_buzzer = millis();  
               if(customKey == 'A' || customKey == 'B')
               {
                 //Botao voltar, volta para aguardar o valor anterior.
                 if(customKey =='A')
                 {
                   lcd2.setCursor(1,2);
                   lcd2.print(F("_"));
                   controle_vmc=38;
                 //Botao OK, automaticamente completa os campos, jogando os valores digitados para ultimas casas. 
                 }else{
                         lcd2.noBlink();
                         lcd2.setCursor(0,2);
                         lcd2.print(F("00"));
                         lcd2.setCursor(2,2);
                         lcd2.write(config_preco_valor[0]);
                         lcd2.setCursor(3,2);
                         lcd2.write(config_preco_valor[1]);
                         config_preco_valor[2] = config_preco_valor[0];
                         config_preco_valor[3] = config_preco_valor[1];
                         for(int i=0; i<2;i++)
                         {
                           config_preco_valor[i] = 48;
                         }
                         controle_vmc = 41;
                       }
               }else 
               {
                 // Armazena o valor e mostra na tela.  
                 config_preco_valor[2] = customKey;                
                 lcd2.print(customKey);                           
                 controle_vmc++; 
               } 
               teclado.valor_lido();                          
              } 
              break;
   // Aguarda o proximo valor ser digitado no teclado matricial.   
   case 40:
           lcd2.setCursor(3,2);
           customKey = teclado.leitura(); 
             if ( customKey != NO_KEY )
           {
               // Ativa controle do buzzer para emitir o beep.
               controle_buzzer = 1;
               time_start_buzzer = millis();  
               if ( customKey == 'A' || customKey == 'B' )
               {
                 // Botao voltar, volta para aguardar o valor anterior.
                 if ( customKey == 'A' )
                 {
                   lcd2.setCursor(2,2);
                   lcd2.print(F("_"));
                   controle_vmc = 39;
                //Botao OK, automaticamente completa os campos, jogando os valores digitados para ultimas casas.    
                }else{
                         lcd2.noBlink();
                         lcd2.setCursor(0,2);
                         lcd2.print(F("0"));
                         lcd2.setCursor(1,2);
                         lcd2.write(config_preco_valor[0]);
                         lcd2.setCursor(2,2);
                         lcd2.write(config_preco_valor[1]);
                         lcd2.setCursor(3,2);
                         lcd2.write(config_preco_valor[2]);
                         config_preco_valor[3] = config_preco_valor[2];
                         config_preco_valor[2] = config_preco_valor[1];
                         config_preco_valor[1] = config_preco_valor[0]; 
                         config_preco_valor[0] = 48;                        
                         controle_vmc++;
                       }
               }else {
                 //Armazena o valor e mostra na tela.  
                 config_preco_valor[3] = customKey;                
                 lcd2.print(customKey); 
                 lcd2.noBlink();               
                 controle_vmc++; 
               }  
               teclado.valor_lido();                         
              } 
           break;    
   // Apos dados selecionados, mostra a confirmacao. 
   case 41:
          lcd2.setCursor(0,3);
          lcd2.print(F("CONFIRMA? OK /  "));
          lcd2.setCursor(15,3);
          lcd2.write(3);  
          controle_vmc++;  
          break;
   // Aguarda confirmacao ou voltar.              
   case 42:
          customKey = teclado.leitura(); 
          if ( customKey != NO_KEY )
          {
               //Ativa controle do buzzer para emitir o beep.
                controle_buzzer=1;
                time_start_buzzer=millis();  
               if(customKey== 'A' || customKey == 'B')
               {
                 //Botao voltar, volta para aguardar o ultimo valor digitado.
                 if(customKey =='A')
                 {
                   lcd2.blink();
                   lcd2.setCursor(3,2);                      
                   lcd2.print(F("_"));
                   lcd2.setCursor(0,3);
                   lcd2.print(F("                   "));
                   lcd2.setCursor(0,3);
                   lcd2.print(F("CORRIGIR  "));
                   lcd2.setCursor(10,3);
                   lcd2.write(3);
                   controle_vmc=31;
                   break;
                 }
                 //Botao OK, prosseguir para armazenar o dado.
                 if(customKey =='B')
                 {
                   controle_vmc++;
                 }
               }
               teclado.valor_lido();                                                             
              }                    
           break;  
   //Calculo para transformar o dados recebidos em um unico dado.        
   case 43:
           mult=1000;
           valor_real=0;
           //Logica para calcular os dados e transformá-los em um inteiro.
           for(int i=0; i<4;i++)
          {
            config_preco_valor[i] = (config_preco_valor[i] - 48)*mult;
            valor_real  = valor_real + config_preco_valor[i];            
            mult= mult/10;
          } 
          estoque =  valor_real; 
          escreve_eeprom(1001,1002,estoque);
          lcd2.clear();  
          lcd2.setCursor(0,1);
          lcd2.print(F("ESTOQUE ALTERADO"));
          lcd2.setCursor(0,2);
          lcd2.print(F("COM SUCESSO!"));    
          time_start_msg = millis();
          controle_vmc++;
          break; 
     case 44:
          tempo_atual_msg = millis();
          if((tempo_atual_msg-time_start_msg) > 2000)
          {
            controle_vmc = 81;            
          } 
          break;  
 //----------------------------RELATORIO DE VENDAS--------------------------------
     case 45:
            menu_relatorio();
            controle_vmc++;
            break; 
      //Percorre o menu atravez do teclado matricial.        
      case 46:                  
             customKey = teclado.leitura(); 
             if ( customKey != NO_KEY )
             {
               //Ativa controle do buzzer para emitir o beep.
                controle_buzzer=1;
                time_start_buzzer=millis();
                time_start_retorna = millis();
               switch(customKey)
               {
                 //Botao 8 para cima.
                 case '8':
                         if(posicao<3){
                            posicao++;
                         }else{
                           posicao=0; 
                          } 
                          controle_vmc = 45;
                          break;
                 //Botao 2 para baixo.         
                 case '2':
                          if(posicao>0){                          
                            posicao--;
                          }else{
                           posicao=3; 
                          } 
                          controle_vmc = 45;
                          break;
                 //Botao OK, entra no menu selecionado.         
                 case 'B':
                          switch(posicao)
                          {
                            //ATIVA
                            case 0:                     
                                   controle_vmc = 47;
                                   break;
                            //PERMANENTE.      
                            case 1:                     
                                   controle_vmc = 49;
                                   break;
                            //RESETA.     
                            case 2:                    
                                  controle_vmc = 51;
                                  break;    
                            //FALHA VENDA.     
                            case 3:         
                                  if( qtd_eventos_falha > 0 )
                                  {           
                                    controle_vmc = 85;
                                    lcd2.setCursor(0,3);
                                    lcd2.print(F("DETALHAR OK /     "));
                                    lcd2.setCursor(14,3);
                                    lcd2.write(3); 
                                  }else{
                                    lcd2.setCursor(0,1);
                                    lcd2.print(F("NENHUM EVENTO      "));
                                    lcd2.setCursor(0,2);
                                    lcd2.print(F("DE ERRO."));
                                    lcd2.setCursor(0,3);
                                    lcd2.print(F("                   "));
                                    time_start_msg = millis();
                                    controle_vmc = 89;
                                  }                                   
                                  break;            
                          }
                          posicao = 0;
                          break;
                   //Botao voltar, volta para o menu central.         
                   case 'A':         
                          controle_vmc = 21;
                          posicao = 0;
                          break;
                 }
                 teclado.valor_lido();
             }                   
            break;
      case 89:
          tempo_atual_msg = millis();
          if((tempo_atual_msg-time_start_msg) > 2000)
          {
            controle_vmc = 45;          
          } 
          break;       
//---------------------------FALHA VENDA-------------            
      case 85:
            visualiza_evento_falha();
            controle_vmc++;
            break; 
      //Percorre o menu atravez do teclado matricial.        
      case 86:                  
             customKey = teclado.leitura(); 
             if ( customKey != NO_KEY )
             {
               //Ativa controle do buzzer para emitir o beep.
               controle_buzzer=1;
               time_start_buzzer=millis();
               time_start_retorna = millis();
               switch(customKey)
               {
                 //Botao 8 para cima.
                 case '8':
                         if( posicao < controle_dez_eventos-1 )                         
                            posicao++;
                         else
                            posicao=0; 
                          
                          controle_vmc = 85;
                          break;
                 //Botao 2 para baixo.         
                 case '2':
                          if ( posicao > 0 ){                          
                            posicao--;
                          }else{
                           posicao = controle_dez_eventos-1; 
                          } 
                          
                          controle_vmc = 85;
                          break; 
                  case 'B':         
                          controle_vmc = 87;
                          break;                
                   //Botao voltar, volta para o menu central.         
                   case 'A':         
                          controle_vmc = 45;
                          posicao = 0;
                          break;
                 }
                 teclado.valor_lido();
             }                   
            break;
     case 87:
            detail_evento_falha();
            controle_vmc++;
            break;
     case 88:
            customKey = teclado.leitura(); 
             if ( customKey != NO_KEY )
            {
              //Ativa controle do buzzer para emitir o beep.
              controle_buzzer=1;
              time_start_buzzer=millis();
              
              switch(customKey)
              {
                case 'B':         
                      lcd2.clear();
                      lcd2.setCursor(0,0);
                      lcd2.print(F("MENU 01.4"));	
                      lcd2.setCursor(0,3);
                      lcd2.print(F("DETALHAR OK /      "));
                      lcd2.setCursor(14,3);
                      lcd2.write(3); 
                      controle_vmc = 85;
                      break;                
                //Botao voltar, volta para o menu central.         
                case 'A': 
                      lcd2.clear();
                      lcd2.setCursor(0,0);
                      lcd2.print(F("MENU 01.4"));	
                      lcd2.setCursor(0,3);
                      lcd2.print(F("DETALHAR OK /      "));
                      lcd2.setCursor(14,3);
                      lcd2.write(3);       
                      controle_vmc = 85;
                      break;
              } 
              teclado.valor_lido();
            }
            break;     
 //--------------------------ATIVA-------------------------------------------
     case 47:  
            lcd2.clear();
            printDate_relatorio(1);
            lcd2.setCursor(0,1);            
            lcd2.print(F("QTD MOEDAS DISPENS.:"));
            lcd2.setCursor(0,2);            
            lcd2.print(F("                    "));
            if ( valor_total_inserido < 10 )
            {
              lcd2.setCursor(7,2);            
              lcd2.print(F("00000"));
              lcd2.setCursor(12,2);            
              lcd2.print(valor_total_inserido);
            }
            if ( valor_total_inserido >= 10 && valor_total_inserido < 100 )
            {
              lcd2.setCursor(7,2);            
              lcd2.print(F("0000"));
              lcd2.setCursor(11,2);            
              lcd2.print(valor_total_inserido);
            }
            if ( valor_total_inserido >= 100 && valor_total_inserido < 1000 ) 
            {
              lcd2.setCursor(7,2);            
              lcd2.print(F("000"));
              lcd2.setCursor(10,2);            
              lcd2.print(valor_total_inserido);
            }
            if ( valor_total_inserido >= 1000 && valor_total_inserido < 10000 )
            {
              lcd2.setCursor(7,2);            
              lcd2.print(F("00"));
              lcd2.setCursor(9,2);            
              lcd2.print(valor_total_inserido);
            }
            if ( valor_total_inserido >= 10000 && valor_total_inserido<100000)
            {
              lcd2.setCursor(7,2);            
              lcd2.print(F("0"));
              lcd2.setCursor(8,2);            
              lcd2.print(valor_total_inserido);
            }
            if ( valor_total_inserido >= 100000 )
            {
              lcd2.setCursor(7,2);            
              lcd2.print(valor_total_inserido);
            }           
              lcd2.setCursor(0,3);    
              lcd2.print(F("RECEITA: R$ "));
              divide_int = receita_total;  
              if(divide_int <10)
              {
                lcd2.setCursor(12,3);            
                lcd2.print(F("0000"));
                lcd2.setCursor(16,3);            
                lcd2.print(divide_int);
              }
              if ( divide_int >= 10 && divide_int < 100 )
              {
                lcd2.setCursor(12,3);            
                lcd2.print(F("000"));
                lcd2.setCursor(15,3);            
                lcd2.print(divide_int);
              }
              if ( divide_int >= 100 && divide_int < 1000 )
              {
                lcd2.setCursor(12,3);            
                lcd2.print(F("00"));
                lcd2.setCursor(14,3);            
                lcd2.print(divide_int);
              }
              if ( divide_int >= 1000 && divide_int < 10000 )
              {
                lcd2.setCursor(12,3);            
                lcd2.print(F("0"));
                lcd2.setCursor(13,3);            
                lcd2.print(divide_int);
              }
              if ( divide_int >= 10000 && divide_int < 100000 )
              {
                lcd2.setCursor(12,3);            
                lcd2.print(divide_int);
              }
              lcd2.setCursor(17,3);            
              lcd2.print(F(",00"));              
              controle_vmc++;
              break; 
    case 48:
            customKey = teclado.leitura(); 
             if ( customKey != NO_KEY )
            {
              //Ativa controle do buzzer para emitir o beep.
              controle_buzzer = 1;
              time_start_buzzer = millis();  
              switch(customKey)
              {
                case 'A':
                        controle_vmc = 45;
                        break;
                case 'B':
                        controle_vmc = 45;
                        break;
              }
              teclado.valor_lido();
            }
            break;
    case 49:  
            lcd2.clear();
            printDate_relatorio(2);
            lcd2.setCursor(0,1);            
            lcd2.print(F("QTD MOEDAS DISPENS.:"));
            lcd2.setCursor(0,2);            
            lcd2.print(F("                    "));
            if ( i_valor_total_inserido < 10 )
            {
              lcd2.setCursor(7,2);            
              lcd2.print(F("00000"));
              lcd2.setCursor(12,2);            
              lcd2.print(i_valor_total_inserido);
            }
            if ( i_valor_total_inserido >= 10 && i_valor_total_inserido < 100 )
            {
              lcd2.setCursor(7,2);            
              lcd2.print(F("0000"));
              lcd2.setCursor(11,2);            
              lcd2.print(i_valor_total_inserido);
            }
            if ( i_valor_total_inserido >= 100 && i_valor_total_inserido < 1000 )
            {
              lcd2.setCursor(7,2);            
              lcd2.print(F("000"));
              lcd2.setCursor(10,2);            
              lcd2.print(i_valor_total_inserido);
            }
            if ( i_valor_total_inserido >= 1000 && i_valor_total_inserido < 10000 )
            {
              lcd2.setCursor(7,2);            
              lcd2.print(F("00"));
              lcd2.setCursor(9,2);            
              lcd2.print(i_valor_total_inserido);
            }
            if ( i_valor_total_inserido >= 10000 && i_valor_total_inserido < 100000 )
            {
              lcd2.setCursor(7,2);            
              lcd2.print(F("0"));
              lcd2.setCursor(8,2);            
              lcd2.print(i_valor_total_inserido);
            }
            if ( i_valor_total_inserido >= 100000 )
            {
              lcd2.setCursor(7,2);            
              lcd2.print(i_valor_total_inserido);
            }
            lcd2.setCursor(0,3);    
            lcd2.print(F("RECEITA: R$ "));
            divide_int = i_receita_total;
            if ( divide_int < 10 )
            {
              lcd2.setCursor(12,3);            
              lcd2.print(F("0000"));
              lcd2.setCursor(16,3);            
              lcd2.print(divide_int);
            }
            if ( divide_int >= 10 && divide_int < 100 )
            {
              lcd2.setCursor(12,3);            
              lcd2.print(F("000"));
              lcd2.setCursor(15,3);            
              lcd2.print(divide_int);
            }
            if ( divide_int >= 100 && divide_int < 1000 )
            {
              lcd2.setCursor(12,3);            
              lcd2.print(F("00"));
              lcd2.setCursor(14,3);            
              lcd2.print(divide_int);
            }
            if ( divide_int >= 1000 && divide_int < 10000 )
            {
              lcd2.setCursor(12,3);            
              lcd2.print(F("0"));
              lcd2.setCursor(13,3);            
              lcd2.print(divide_int);
            }
            if ( divide_int >= 10000 && divide_int < 100000 )
            {
              lcd2.setCursor(12,3);            
              lcd2.print(divide_int);
            }
            lcd2.setCursor(17,3);            
            lcd2.print(F(",00"));              
            controle_vmc++;
            break; 
    case 50:
            customKey = teclado.leitura(); 
             if ( customKey != NO_KEY )
            {
              //Ativa controle do buzzer para emitir o beep.
              controle_buzzer = 1;
              time_start_buzzer = millis();  
              switch(customKey)
              {
                case 'A':
                        controle_vmc = 45;
                        break;
                case 'B':
                        controle_vmc = 45;
                        break;
              }
              teclado.valor_lido();
            }
            break;   
     case 51:
          lcd2.clear(); 
          lcd2.setCursor(0,0);            
          lcd2.print(F("TEM CERTEZA QUE"));
          lcd2.setCursor(0,1);            
          lcd2.print(F("DESEJA RESETAR O"));
          lcd2.setCursor(0,2);            
          lcd2.print(F("RELATORIO DE VENDAS?"));
          lcd2.setCursor(0,3);
          lcd2.print(F("CONFIRMA OK /  "));
          lcd2.setCursor(14,3);
          lcd2.write(3); 
          controle_vmc++;
          break;
    case 52:
          customKey = teclado.leitura(); 
             if ( customKey != NO_KEY )
          {
            //Ativa controle do buzzer para emitir o beep.
            controle_buzzer = 1;
            time_start_buzzer = millis();  
            switch(customKey)
            {
              case 'A':
                      controle_vmc = 45;
                      break;
              case 'B':                      
                      valor_total_inserido = 0;
                      receita_total = 0;
                      
                      escreve_eeprom(EEPROM_ADDR_VALOR_TOTAL_1, EEPROM_ADDR_VALOR_TOTAL_2, valor_total_inserido);
                      escreve_eeprom(EEPROM_ADDR_RECEITA_TOTAL_1, EEPROM_ADDR_RECEITA_TOTAL_2, receita_total);
                      controle_vmc++;
                      break;
               case '9':
                      valor_total_inserido = 0;
                      i_valor_total_inserido = 0;
                      receita_total = 0;
                      i_receita_total = 0;
                      
                      escreve_eeprom(EEPROM_ADDR_VALOR_TOTAL_1, EEPROM_ADDR_VALOR_TOTAL_2, valor_total_inserido);
                      escreve_eeprom(EEPROM_ADDR_I_VALOR_TOTAL_1, EEPROM_ADDR_I_VALOR_TOTAL_2, i_valor_total_inserido);
                      escreve_eeprom(EEPROM_ADDR_RECEITA_TOTAL_1, EEPROM_ADDR_RECEITA_TOTAL_2, receita_total);
                      escreve_eeprom(EEPROM_ADDR_I_RECEITA_TOTAL_1, EEPROM_ADDR_I_RECEITA_TOTAL_2, i_receita_total);  
                      break;           
            }
            teclado.valor_lido();
          }
          break;
   case 53:
          lcd2.clear(); 
          lcd2.setCursor(0,0);            
          lcd2.print(F("RELATORIOS DE VENDAS"));
          lcd2.setCursor(0,1);            
          lcd2.print(F("      RESETADO"));
          lcd2.setCursor(0,2); 
          lcd2.print(F("    COM SUCESSO!"));
          time_start= millis();
          controle_vmc++;
          break;       
   case 54:
          tempo_atual= millis();
           if((tempo_atual-time_start) > 2000)
            {
                time_start = tempo_atual;  
                controle_vmc=45;  
            }                   
           break;       
//-------------------TESTES---------------------           
      case 55:
            menu_testes();
            controle_vmc++;
            break; 
      //Percorre o menu atravez do teclado matricial.        
      case 56:                  
             customKey = teclado.leitura(); 
             if ( customKey != NO_KEY )
             {
               //Ativa controle do buzzer para emitir o beep.
               controle_buzzer = 1;
               time_start_buzzer = millis();
               time_start_retorna = millis();
               switch(customKey)
               {
                 //Botao 8 para cima.
                 case '8':
                         if(posicao<0){
                            posicao++;
                         }else{
                           posicao=0; 
                          } 
                          controle_vmc = 55;
                          break;
                 //Botao 2 para baixo.         
                 case '2':
                          if(posicao>0){                          
                            posicao--;
                          }else{
                           posicao=0; 
                          } 
                          controle_vmc = 55;
                          break;
                 //Botao OK, entra no menu selecionado.         
                 case 'B':
                          switch(posicao)
                          {
                            //TESTE ENTREGA
                            case 0:                     
                                   controle_vmc = 57;
                                   break;
                            //DISPENSA MANUAL     
                            case 1:                     
                                   //controle_vmc = 63;
                                   break;
                            //LDR     
                            case 2:                    
                                  //controle_vmc = 51;
                                  break;               
                          }
                          posicao = 0;
                          break;
                   //Botao voltar, volta para o menu central.         
                   case 'A':         
                          controle_vmc = 21;
                          posicao = 0;
                          break;
                 }
                 teclado.valor_lido();
             }                   
            break; 
//-------------------TESTA ENTREGA--------------------------            
      case 57:
          lcd2.clear(); 
          lcd2.setCursor(0,1);            
          lcd2.print(F("  AGUARDE ENQUANTO"));
          lcd2.setCursor(0,2);            
          lcd2.print(F("O TESTE E REALIZADO"));
          teste_entrega = 1;
          qtd_moedas_dispensar = 1;         
          timeout_motor = 21000;
          
          digitalWrite(RELE_2,HIGH);
          
          sensor_queda_infra.realiza_leitura();
          sensor_queda_infra.set_evento_disponivel(0);   
          contador_moedas = 0;
          leitura_rep = 1;
          time_start_ldr = millis();
          time_start_timeout_motor = millis();
          controle_timeout_motor = 1;              
          
          controle_vmc = 100;
          break;
      case 100:          
          // Verifica Timeout de 15 segundos caso não constate verificação do sensor da base.
          tempo_atual_infra = millis();
          
          // ✅ CORREÇÃO CRÍTICA 1: Adiciona timeout de 10 segundos
          if ((tempo_atual_infra - time_start_infra) > 10000) {
            // TIMEOUT - Produto não caiu
            Serial.println(F("*** TIMEOUT: Produto não caiu ***"));
            
            // Desliga motor
            digitalWrite(RELE_2, LOW);
            
            // Finaliza leitura do sensor
            sensor_queda_infra.finaliza_leitura();
            
            // Registra erro na EEPROM
            qtd_eventos_falha++;
            escreve_eeprom(EEPROM_ADDR_QTD_EVENTOS_FALHA_1, 
                           EEPROM_ADDR_QTD_EVENTOS_FALHA_2, 
                           qtd_eventos_falha);
            
            // Retorna dinheiro ao usuário
            if (valor_inserido > 0) {
              Serial.print(F("Retornando R$"));
              Serial.println(valor_inserido / 100);
              mdb.entregar_troco(valor_inserido);
              valor_inserido = 0;
            }
            
            // Mostra mensagem de erro
            lcd2.clear();
            lcd2.setCursor(0,0);
            lcd2.print(F("ERRO: Produto nao"));
            lcd2.setCursor(0,1);
            lcd2.print(F("liberado. Dinheiro"));
            lcd2.setCursor(0,2);
            lcd2.print(F("retornado."));
            
            delay(3000);
            controle_vmc = 0;  // Volta para IDLE
            break;
          }
          
          if((tempo_atual_infra-time_start_infra) > 200)
          {  
            if ( limpa_registro )
            {
              sensor_queda_infra.set_evento_disponivel(0);  
              limpa_registro = 0;
            }
            if ( sensor_queda_infra.get_evento_disponivel() )
            {             
              Serial.print("MOEDA DISPENSADA. Canal Leitura: ");       
              Serial.println(sensor_queda_infra.get_canal_detectado());              
              sensor_queda_infra.finaliza_leitura();     
              sensor_queda_infra.set_evento_disponivel(0);   
              time_start_infra = millis();	
              limpa_registro = 1;
              controle_vmc = 58;
              digitalWrite(RELE_2,LOW);
            }    
          }          
          break;      
      case 58:
          lcd2.clear(); 
          lcd2.setCursor(0,0);            
          lcd2.print(F("  TESTE REALIZADO"));
          lcd2.setCursor(0,1);            
          lcd2.print(F("COM SUCESSO, RETIRE"));
          lcd2.setCursor(0,2); 
          lcd2.print(F("  A MOEDA NA SAIDA"));
          lcd2.setCursor(0,3); 
          lcd2.print(F("     INDICADA."));
          time_start_teste_entrega = millis();
          controle_vmc++;
          break;       
     case 59:
          tempo_atual_teste_entrega= millis();
           if((tempo_atual_teste_entrega-time_start_teste_entrega) > 3500)
            {              
              controle_vmc = 81;  
            }                   
           break;
    case 81:
          lcd2.clear(); 
          lcd2.setCursor(0,0);            
          lcd2.print(F("     A MAQUINA"));
          lcd2.setCursor(0,1);            
          lcd2.print(F(" SERA REINICIADA EM:"));          
          lcd2.setCursor(0,2); 
          lcd2.print(F("     3 SEGUNDOS,"));
          lcd2.setCursor(0,3); 
          lcd2.print(F("      AGUARDE."));
          time_start_teste_entrega = millis();
          cont_segundos = 3;
          controle_vmc++;
          break;       
     case 82:
          tempo_atual_teste_entrega= millis();
          if((tempo_atual_teste_entrega-time_start_teste_entrega) > 1000)
          {
            if(cont_segundos == 3)
            {
             lcd2.setCursor(0,2); 
             lcd2.print(F("     2 SEGUNDOS,"));
             cont_segundos = 2;
            }
          }
         if((tempo_atual_teste_entrega-time_start_teste_entrega) > 2000)
          {
            if(cont_segundos == 2)
            {
             lcd2.setCursor(0,2); 
             lcd2.print(F("     1 SEGUNDOS,"));
             cont_segundos = 1;
            }
          }
          if((tempo_atual_teste_entrega-time_start_teste_entrega) > 2900)
          {
             if(cont_segundos == 1)
            {
             lcd2.setCursor(0,2); 
             lcd2.print(F("     0 SEGUNDOS,"));
             cont_segundos = 0;
            }
          }   
          if((tempo_atual_teste_entrega-time_start_teste_entrega) > 3000)
          {
            status_vmc = 1;
            EEPROM.write(EEPROM_ADDR_STATUS_VMC, status_vmc);                
            teste_entrega = 0; 
            softReset();
          }                   
          break;       
    case 60:
          lcd2.clear(); 
          lcd2.setCursor(0,1);            
          lcd2.print(F("   ERRO NO TESTE"));
          lcd2.setCursor(0,2);            
          lcd2.print(F("  TENTE NOVAMENTE!"));
          time_start= millis();
          controle_vmc++;
          break;       
     case 61:
          tempo_atual= millis();
           if((tempo_atual-time_start) > 2000)
            {
                time_start = tempo_atual; 
                status_vmc = 0;
                EEPROM.write(EEPROM_ADDR_STATUS_VMC, status_vmc);  
                teste_entrega = 0;  
                controle_vmc=55;  
            }                   
           break;       
//-------------------CONFIG RELOGIO---------------------          
      case 63:
            //Mostra o menu de acordo com o valor de posicao_horario.
            menu_horario();
            controle_vmc++;
            break;
     //Percorre o menu atravez do teclado matricial.       
     case 64:  
           customKey = teclado.leitura(); 
             if ( customKey != NO_KEY )
             {
           //Ativa controle do buzzer para emitir o beep.
           controle_buzzer=1;
           time_start_buzzer=millis();
            switch(customKey)
            {
               //Botao 8 para cima.
               case '8':
                       if(posicao_horario<1)
                       {
                          posicao_horario++;
                       }else
                        {
                         posicao_horario=0; 
                        } 
                        controle_vmc = 63;
                        break;
               //Botao 2 para baixo.           
               case '2':
                        if(posicao_horario>0)
                        {                          
                          posicao_horario--;
                        }else
                        {
                         posicao_horario=1; 
                        } 
                        controle_vmc = 63;
                        break;
               //Botao OK, entra no menu selecionado.          
               case 'B':
                        switch(posicao_horario)
                        {
                          //Configuracao de Hora.
                          case 0:                    
                                  controle_vmc=65;
                                  break;
                          //Configuracao de Data.         
                          case 1:
                                  controle_vmc=72;
                                  break;  
                        }  
                        break;
               //Botao voltar, volta para o menu de servico.              
               case 'A':         
                      controle_vmc = 21;
                      break;           
            }
            teclado.valor_lido();
           }           
           break; 
 //---------CONFIGURACAO DE HORA------------------------  
  //Configuracao da hora da maquina.
   case 65:  
          //Visualizacao do campo de edicao.   
          lcd2.clear();
          lcd2.setCursor(0,0);
          lcd2.print(F("MENU 04.1.1"));
          lcd2.setCursor(0,1);
          lcd2.print(F("CONFIG. HORA: 24Hrs"));  
          lcd2.setCursor(0,3);
          lcd2.print(F("VOLTAR  "));
          lcd2.setCursor(7,3);
          lcd2.write(3);
          lcd2.setCursor(0,2);                      
          lcd2.print(F("__:__")); 
          lcd2.setCursor(0,3);         
          lcd2.print(F("VOLTAR  "));
          lcd2.setCursor(7,3);
          lcd2.write(3);
          controle_vmc++;
          break;
  //Aguarda o primeiro valor ser digitado no teclado matricial.        
  case 66:
        lcd2.blink();
        lcd2.setCursor(0,2);
        customKey = teclado.leitura(); 
             if ( customKey != NO_KEY )
             {
            //Ativa controle do buzzer para emitir o beep.
             controle_buzzer=1;
             time_start_buzzer=millis();
             if(customKey== 'A' || customKey == 'B')
             {
               //Botao voltar, volta para o menu horario.
               if(customKey =='A')
               {
                 lcd2.noBlink();
                 controle_vmc = 63;
                 break;
               }
             }
             //Verifica se valor se encaixa em valores possiveis para hora.
             if(customKey== '1' || customKey == '2'|| customKey == '0')  
             {
               //Armazena o valor e mostra na tela.
               hora_1 = customKey;                
               lcd2.print(customKey);
               lcd2.setCursor(0,3);
               lcd2.print(F("                   "));
               lcd2.setCursor(0,3);
               lcd2.print(F("CORRIGIR  "));
               lcd2.setCursor(10,3);
               lcd2.write(3);
               controle_vmc++;    
             }    
             teclado.valor_lido();                                                
            }                    
         break; 
   //Aguarda o proximo valor ser digitado no teclado matricial.      
   case 67:
          lcd2.setCursor(1,2);
          customKey = teclado.leitura(); 
             if ( customKey != NO_KEY )
             {
               //Ativa controle do buzzer para emitir o beep.
               controle_buzzer=1;
               time_start_buzzer=millis();
               if(customKey== 'A' || customKey == 'B')
               {
                 //Botao voltar, volta para aguardar o valor anterior.
                 if(customKey =='A')
                 {
                   lcd2.setCursor(0,3);
                   lcd2.print(F("                   "));
                   lcd2.setCursor(0,3);
                   lcd2.print(F("VOLTAR  "));
                   lcd2.setCursor(7,3);
                   lcd2.write(3);
                   lcd2.setCursor(0,2);                      
                   lcd2.print(F("_"));
                   controle_vmc=66;
                   break;
                 //Botao OK, automaticamente completa os campos, jogando os valores digitados para ultimas casas.   
                 }else{
                         lcd2.noBlink();
                         lcd2.setCursor(0,2);
                         lcd2.print(F("0"));
                         lcd2.setCursor(1,2);
                         lcd2.write(hora_1);
                         lcd2.setCursor(3,2);
                         lcd2.print(F("00"));
                         hora_2 = hora_1;
                         hora_1 = 48;
                         minuto_1 = 48;
                         minuto_2 = 48;
                         controle_vmc=70;
                         break;
                       }
               }
               //Se o primeiro valor de hora for 2.
               if(hora_1=='2')
               {
                //Verifica se valor se encaixa em valores possiveis unidade de hora.
                if(customKey== '1' || customKey == '2' || customKey== '3' || customKey== '0')
                {
                   //Armazena o valor e mostra na tela. 
                   hora_2 = customKey;                    
                   lcd2.print(customKey);                                                    
                   controle_vmc++;                                                                                     
                }
               //Caso contrario. 
               }else{
                       //Armazena o valor e mostra na tela.
                       hora_2 = customKey;                        
                       lcd2.print(customKey);                                                    
                       controle_vmc++;
                     }
              teclado.valor_lido();
          }                    
           break; 
   //Aguarda o proximo valor ser digitado no teclado matricial.         
   case 68:
          lcd2.setCursor(3,2);
          customKey = teclado.leitura(); 
             if ( customKey != NO_KEY )
             {
              //Ativa controle do buzzer para emitir o beep.
               controle_buzzer=1;
               time_start_buzzer=millis();
               if(customKey== 'A' || customKey == 'B')
               {
                 //Botao voltar, volta para aguardar o valor anterior.
                 if(customKey =='A')
                 {
                   lcd2.setCursor(1,2);                      
                   lcd2.print(F("_"));
                   controle_vmc=67;
                   break;
                 //Botao OK, automaticamente completa os campos, jogando os valores digitados para ultimas casas.   
                 }else{
                         lcd2.noBlink();
                         lcd2.setCursor(3,2);
                         lcd2.print(F("00"));
                         minuto_1 = 48;
                         minuto_2 = 48;
                         controle_vmc=70;
                         break;
                       }
               } 
               //Verifica se valor se encaixa em valores possiveis para minuto.
               if(customKey== '0' || customKey == '1' || customKey== '2' || customKey== '3' || customKey== '4' || customKey== '5')
               {     
                 //Armazena o valor e mostra na tela.                 
                 minuto_1 = customKey;                  
                 lcd2.print(customKey);                                                    
                 controle_vmc++;  
               }      
               teclado.valor_lido();                           
              }                    
           break;
   //Aguarda o proximo valor ser digitado no teclado matricial.         
   case 69:
          lcd2.setCursor(4,2);
          customKey = teclado.leitura(); 
          if ( customKey != NO_KEY )
          {
               //Ativa controle do buzzer para emitir o beep.
               controle_buzzer=1;
               time_start_buzzer=millis();
               if(customKey== 'A' || customKey == 'B')
               {
                 //Botao voltar, volta para aguardar o valor anterior.
                 if(customKey =='A')
                 {
                   lcd2.setCursor(3,2);                      
                   lcd2.print(F("_"));
                   controle_vmc=68;
                   break;
                 //Botao OK, automaticamente completa os campos, jogando os valores digitados para ultimas casas.  
                 }else{
                         lcd2.noBlink();
                         lcd2.setCursor(4,2);
                         lcd2.print(F("0"));
                         minuto_2 = 48;
                         controle_vmc=70;
                         break;
                       }
               }       
               //Armazena o valor e mostra na tela.               
               minuto_2 = customKey;                
               lcd2.print(customKey); 
               lcd2.noBlink();               
               controle_vmc++;  
               teclado.valor_lido();                                                            
              }                    
           break;
   //Apos dados selecionados, mostra a confirmacao.        
   case 70:
          lcd2.setCursor(0,3);
          lcd2.print(F("CONFIRMA? OK /  "));
          lcd2.setCursor(15,3);
          lcd2.write(3);  
          controle_vmc++;  
          break;
   //Aguarda confirmacao ou voltar.        
   case 71:
          customKey = teclado.leitura(); 
          if ( customKey != NO_KEY ) 
          {
             //Ativa controle do buzzer para emitir o beep.
             controle_buzzer=1;
             time_start_buzzer=millis();
              switch(customKey)
              {
                  //Botao voltar, volta para aguardar o ultimo valor digitado.
                  case 'A':
                      lcd2.blink();
                      lcd2.setCursor(0,3);
                      lcd2.print(F("                   "));
                      lcd2.setCursor(0,3);
                      lcd2.print(F("CORRIGIR  "));
                      lcd2.setCursor(10,3);
                      lcd2.write(3);
                      lcd2.setCursor(4,2);                      
                      lcd2.print(F("_"));
                      controle_vmc=69;
                      break;
                  //Botao OK, armazenar o valor no dispositivo.     
                  case 'B':
                        hora_1 = (hora_1-48)*10;
                        hora_2 = hora_2-48;
                        hora_1 = hora_1 + hora_2;
                        minuto_1 = (minuto_1-48)*10;
                        minuto_2 = minuto_2-48;
                        minuto_1 = minuto_1 + minuto_2;
                        
                        DateTime tstamp = rtc.now();
                        rtc.adjust(DateTime(tstamp.year(), tstamp.month(), tstamp.day(), hora_1, minuto_1, 0));
                        
                        //Volta para menu horario.
                        controle_vmc = 63; 
              }
              teclado.valor_lido();
          }                            
          break;
//-------------------CONFIGURACAO DATA------------------------
    //Configuracao de data da maquina.
    case 72:
          //Visualizacao do campo de edicao.
          lcd2.clear();
          lcd2.setCursor(0,0);
          lcd2.print(F("MENU 04.1.2"));
          lcd2.setCursor(0,1);
          lcd2.print(F("CONF. DATA: DD/MM/AA"));  
          lcd2.setCursor(0,2);                      
          lcd2.print(F("__/__/__")); 
          lcd2.setCursor(0,3);         
          lcd2.print(F("VOLTAR  "));
          lcd2.setCursor(7,3);
          lcd2.write(3);
          controle_vmc++;
          break; 
   //Aguarda o primeiro valor ser digitado no teclado matricial.        
   case 73:
          lcd2.blink();
          lcd2.setCursor(0,2);
          customKey = teclado.leitura(); 
          if ( customKey != NO_KEY )
          {
               //Ativa controle do buzzer para emitir o beep.
               controle_buzzer=1;
               time_start_buzzer=millis();
               if(customKey== 'A' || customKey == 'B')
               {
                 //Botao voltar, volta para o menu horario.
                 if(customKey =='A')
                 {
                   lcd2.noBlink();
                   controle_vmc = 63;
                   break;
                 }
               }
               //Verifica se valor se encaixa em valores possiveis para dia.
               if(customKey== '1' || customKey == '2'|| customKey == '3' || customKey == '0')  
               {
                 //Armazena o valor e mostra na tela.
                 dia_1 = customKey;                  
                 lcd2.print(customKey);
                 lcd2.setCursor(0,3);
                 lcd2.print(F("                   "));
                 lcd2.setCursor(0,3);
                 lcd2.print(F("CORRIGIR  "));
                 lcd2.setCursor(10,3);
                 lcd2.write(3);
                 controle_vmc++;    
               }        
               teclado.valor_lido();                                            
              }                    
           break; 
   //Aguarda o proximo valor ser digitado no teclado matricial.         
   case 74:
          lcd2.setCursor(1,2);
          customKey = teclado.leitura(); 
          if ( customKey != NO_KEY )
          {
                //Ativa controle do buzzer para emitir o beep.
                 controle_buzzer=1;
                 time_start_buzzer=millis();
               if(customKey== 'A' || customKey == 'B')
               {
                 //Botao voltar, volta para aguardar o valor anterior.
                 if(customKey =='A')
                 {
                   lcd2.setCursor(0,3);
                   lcd2.print(F("                   "));
                   lcd2.setCursor(0,3);
                   lcd2.print(F("VOLTAR  "));
                   lcd2.setCursor(7,3);
                   lcd2.write(3);
                   lcd2.setCursor(0,2);                      
                   lcd2.print(F("_"));
                   controle_vmc=73;
                   break;
                 //Botao OK, automaticamente completa os campos de dia, jogando o valor digitado para ultima casa.
                 //Se dezena do dia diferente de 0.  
                 }else if (dia_1 != '0'){                        
                           lcd2.setCursor(0,2);
                           lcd2.print(F("0"));
                           lcd2.setCursor(1,2);
                           lcd2.write(dia_1);                         
                           dia_2 = dia_1;
                           dia_1 = 48;
                           controle_vmc=79;
                           break;  
                 }                       
               //Logica para armazenar os dados do dia.  
               }else if (dia_1 == '0' || dia_1 == '3'){
                         
                         if(dia_1 == '0')
                         {
                           if (customKey != '0')
                           {     
                               //Armazena o valor e mostra na tela.                      
                               dia_2 = customKey;                    
                               lcd2.print(customKey);                                                    
                               controle_vmc++;  
                           }
                         }else{
                            if (customKey == '1' || customKey == '0')
                           {
                             //Armazena o valor e mostra na tela.
                             dia_2 = customKey;                    
                             lcd2.print(customKey);                                                    
                             controle_vmc++;
                           }
                        }
               }else {
                           //Armazena o valor e mostra na tela.
                           dia_2 = customKey;                    
                           lcd2.print(customKey);                                                    
                           controle_vmc++; 
                       }
                       teclado.valor_lido();
          }                    
           break;
   //Aguarda o proximo valor ser digitado no teclado matricial.          
   case 75:
          lcd2.setCursor(3,2);
          customKey = teclado.leitura(); 
          if ( customKey != NO_KEY )
          {
                //Ativa controle do buzzer para emitir o beep.
                 controle_buzzer=1;
                 time_start_buzzer=millis();
               if(customKey== 'A' || customKey == 'B')
               {
                 //Botao voltar, volta para aguardar o valor anterior.
                 if(customKey =='A')
                 {
                   lcd2.setCursor(1,2);                      
                   lcd2.print(F("_"));
                   controle_vmc=74;
                   break;
                 }
               }  
               //Verifica se valor se encaixa em valores possiveis para mes.
               if(customKey== '0' || customKey == '1')
               {   
                 //Armazena o valor e mostra na tela.                 
                 mes_1 = customKey;                  
                 lcd2.print(customKey);                                                    
                 controle_vmc++;  
               }   
               teclado.valor_lido();                              
              }                    
           break; 
   //Aguarda o proximo valor ser digitado no teclado matricial.         
   case 76:
          lcd2.setCursor(4,2);
          customKey = teclado.leitura(); 
          if ( customKey != NO_KEY )
          {
                //Ativa controle do buzzer para emitir o beep.
                 controle_buzzer=1;
                 time_start_buzzer=millis();
               if(customKey== 'A' || customKey == 'B')
               {
                 //Botao voltar, volta para aguardar o valor anterior.
                 if(customKey =='A')
                 {
                   lcd2.setCursor(3,2);                      
                   lcd2.print(F("_"));
                   controle_vmc=75;
                   break;
                 //Botao OK, automaticamente completa os campos do mes, jogando o valor digitado para ultima casa.    
                 }else if(mes_1!=48){
                         lcd2.setCursor(3,2);
                         lcd2.print(F("0"));
                         lcd2.setCursor(4,2);
                         lcd2.write(mes_1);                         
                         mes_2 = mes_1;
                         mes_1 = 48;
                         controle_vmc=79;
                         break;
                       }
               //Logica para armazenar os dados do dia.        
               }else if (mes_1 == '0'){
                         if (customKey != '0')
                         {       
                             //Armazena o valor e mostra na tela.                     
                             mes_2 = customKey;                    
                             lcd2.print(customKey);                                                    
                             controle_vmc++;  
                         } 
               }else {
                           //Armazena o valor e mostra na tela. 
                           mes_2 = customKey;                    
                           lcd2.print(customKey);                                                    
                           controle_vmc++; 
                       }  
                       teclado.valor_lido();            
              }                    
           break; 
   //Aguarda o proximo valor ser digitado no teclado matricial.         
   case 77:
          lcd2.setCursor(6,2);
          customKey = teclado.leitura(); 
          if ( customKey != NO_KEY )
          {
                //Ativa controle do buzzer para emitir o beep.
                 controle_buzzer=1;
                 time_start_buzzer=millis();
               if(customKey== 'A' || customKey == 'B')
               {
                 //Botao voltar, volta para aguardar o valor anterior.
                 if(customKey =='A')
                 {
                   lcd2.setCursor(4,2);                      
                   lcd2.print(F("_"));
                   controle_vmc=76;
                   break;
                 }                 
               }else{ 
                 //Armazena o valor e mostra na tela.                   
                 ano_1 = customKey;                  
                 lcd2.print(customKey);                                                    
                 controle_vmc++; 
               }     
               teclado.valor_lido();            
              }                    
           break; 
   //Aguarda o proximo valor ser digitado no teclado matricial.            
   case 78:
          lcd2.setCursor(7,2);
          customKey = teclado.leitura(); 
          if ( customKey != NO_KEY )
          {
                //Ativa controle do buzzer para emitir o beep.
                 controle_buzzer=1;
                 time_start_buzzer=millis();
               if(customKey== 'A' || customKey == 'B')
               {
                 //Botao voltar, volta para aguardar o valor anterior.
                 if(customKey =='A')
                 {
                   lcd2.setCursor(6,2);                      
                   lcd2.print(F("_"));
                   controle_vmc=77;
                   break;
                 }
               }else{   
               //Armazena o valor e mostra na tela.                 
               ano_2 = customKey;                
               lcd2.print(customKey); 
               lcd2.noBlink();               
               controle_vmc++;    
               }           
               teclado.valor_lido();    
              }                    
           break;  
    //Apos dados selecionados, mostra a confirmacao.         
    case 79:
          lcd2.setCursor(0,3);
          lcd2.print(F("CONFIRMA? OK /  "));
          lcd2.setCursor(15,3);
          lcd2.write(3);  
          controle_vmc++;  
          break;  
   //Aguarda confirmacao ou voltar.       
   case 80:
          customKey = teclado.leitura(); 
          if ( customKey != NO_KEY )
          {
               //Ativa controle do buzzer para emitir o beep.
               controle_buzzer=1;
               time_start_buzzer=millis();       
              switch(customKey)
              {
                  //Botao voltar, volta para aguardar o ultimo valor digitado.
                  case 'A':
                      lcd2.blink();
                      lcd2.setCursor(0,3);
                      lcd2.print(F("                   "));
                      lcd2.setCursor(0,3);
                      lcd2.print(F("CORRIGIR  "));
                      lcd2.setCursor(10,3);
                      lcd2.write(3);
                      lcd2.setCursor(7,2);                      
                      lcd2.print(F("_"));
                      controle_vmc=78;
                      break;
                  //Botao OK, calcula e armazena o valor no dispositivo.     
                  case 'B':
                        dia_1 = (dia_1-48)*10;
                        dia_2 = dia_2-48;
                        dia_1 = dia_1 + dia_2;
                        mes_1 = (mes_1-48)*10;
                        mes_2 = mes_2-48;
                        mes_1 = mes_1 + mes_2;
                        ano_1 = (ano_1-48)*10;
                        ano_2 = ano_2-48;
                        ano_1 = ano_1 + ano_2;  
                        ano_1 = ano_1 + 2000;    
                        
                        DateTime tstamp = rtc.now();
                        rtc.adjust(DateTime(ano_1, mes_1, dia_1, tstamp.hour(), tstamp.minute(), tstamp.second()));
                        
                        //Volta para menu horario.
                        controle_vmc = 63; 
              }
              teclado.valor_lido();
          }                            
          break;   
//-----------------------------ENTREGA PRODUTO------------------------------------             
      case 1: //xisde
            lcd.clear();
            lcd.setCursor(0,1);
            lcd.print(F(" VALOR INS.: R$5,00"));
            lcd.setCursor(0,2);
            lcd.print(F("   QTD MOEDAS: 1."));  
            time_start_value = millis();          
            controle_vmc++;
            break; 
      case 2:
            tempo_atual_value = millis();
            if((tempo_atual_value-time_start_value) > 2000)
            {
              time_start_value = tempo_atual_value; 
              digitalWrite(RELE_2,HIGH);
              digitalWrite(RELE_1,HIGH);
              /*sensor_queda_infra.realiza_leitura();
              sensor_queda_infra.set_evento_disponivel(0);   
              
              controle_ldr = 1;
              contador_moedas = 0;
              leitura_rep = 1;
              time_start_ldr = millis();
              time_start_timeout_motor = millis();
              controle_timeout_motor = 1;                 
              controle_vmc = 100;
              */
              
              sensor_queda_infra.realiza_leitura();
              sensor_queda_infra.set_evento_disponivel(0);   
              contador_moedas = 0;
              leitura_rep = 1;
              time_start_ldr = millis();
              time_start_timeout_motor = millis();
              controle_timeout_motor = 1;              
              
              controle_vmc = 100;
            } 
            break;
      case 3:      
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print(F("AGUARDE ATE QUE"));
            lcd.setCursor(0,1);
            lcd.print(F("A MOEDA SEJA"));
            lcd.setCursor(0,2);
            lcd.print(F("DISPENSADA PELO"));
            lcd.setCursor(0,3);
            lcd.print(F("LOCAL INDICADO."));   
            controle_vmc=-1;
            break; 
      case 4:
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print(F("MOEDA ENTREGUE"));
            lcd.setCursor(0,1);
            lcd.print(F("COM SUCESSO,"));
            lcd.setCursor(0,2);
            lcd.print(F("RETIRE A NA SAIDA"));
            lcd.setCursor(0,3);
            lcd.print(F("INDICADA, OBRIGADO!"));
            time_start_value = millis();   
            controle_vmc++;
            break;
       case 5:
            tempo_atual_value = millis();
            if((tempo_atual_value-time_start_value) > 3500)
            {
              time_start_value = tempo_atual_value;    
              controle_vmc=0;
            } 
            break; 
       case 6:
            lcd.clear();
            lcd.setCursor(0,1);
            lcd.print(F("VALOR INS.: R$10,00"));
            lcd.setCursor(0,2);
            lcd.print(F("   QTD MOEDAS: 2."));  
            time_start_value = millis();          
            controle_vmc=2;
            break; 
       case 7:
            lcd.clear();
            lcd.setCursor(0,1);
            lcd.print(F("VALOR INS.: R$20,00"));
            lcd.setCursor(0,2);
            lcd.print(F("   QTD MOEDAS: 4."));  
            time_start_value = millis();          
            controle_vmc=2;
            break; 
       case 90:   
            lcd.clear();
            lcd.setCursor(0,1);
            lcd.print(F("VALOR INS.: R$00,00"));
            if(ultimo_valor_inserido < 1000 )
            {
              lcd.setCursor(14,1);
              lcd.print(ultimo_valor_inserido/100);
            }else{
              lcd.setCursor(13,1);
              lcd.print(ultimo_valor_inserido/100);
            }
            lcd.setCursor(0,2);
            lcd.print(F("   QTD MOEDAS: 4.")); 
            lcd.setCursor(15,1);
            lcd.print(ultimo_valor_inserido/500);
            time_start_value = millis();          
            controle_vmc=2;  
            break;
       case 8:
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print(F("MOEDAS ENTREGUES"));
            lcd.setCursor(0,1);
            lcd.print(F("COM SUCESSO,"));
            lcd.setCursor(0,2);
            lcd.print(F("RETIRE-AS NA SAIDA"));
            lcd.setCursor(0,3);
            lcd.print(F("INDICADA, OBRIGADO!"));
            time_start_value = millis();
            
            // ✅ CORREÇÃO CRÍTICA 3: Substituir delay() por código não-bloqueante
            // Mantém MDB ativo durante a espera
            unsigned long timer_fim_venda = millis();
            while (millis() - timer_fim_venda < 100) {
              mdb.task();  // Mantém comunicação MDB ativa
            }
            
            em_venda=0; 
            controle_vmc++;
            break;
       case 9:
            tempo_atual_value = millis();
            if((tempo_atual_value-time_start_value) > 3500)
            {
              em_venda=0; 
              time_start_value = tempo_atual_value;    
              controle_vmc=0;
              //calc_bill_type();
            } 
            break;
      case 10:
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print(F("ERRO AO ENTREGAR"));
            lcd.setCursor(0,1);
            lcd.print(F("MOEDAS, MAQUINA")); 
            lcd.setCursor(0,2);
            lcd.print(F("FICARA INOPERANTE."));    
            time_start_value = millis();   
            controle++;
            break;
     case 11:
            tempo_atual_value = millis();
            if((tempo_atual_value-time_start_value) > 3500)
            {
              time_start_value = tempo_atual_value;    
              status_vmc = 0;
              aux = status_vmc;
              EEPROM.write(EEPROM_ADDR_STATUS_VMC, status_vmc);
              controle_vmc = 0;
            }      
             break;
     case 12:
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print(F("EQUIPAMENTO SEM"));
            lcd.setCursor(0,1);
            lcd.print(F("ESTOQUE SUFICIENTE,")); 
            lcd.setCursor(0,2);
            lcd.print(F("INSIRA UM"));    
            lcd.setCursor(0,3);
            lcd.print(F("VALOR INFERIOR."));    
            time_start_value = millis();   
            controle_vmc++;
            break;
     case 13:
            tempo_atual_value = millis();
            if((tempo_atual_value-time_start_value) > 3500)
            {
              em_venda=0; 
              time_start_value = tempo_atual_value;    
              controle_vmc = 0;
              status_compra = 0;
              //calc_bill_type();              
            }      
             break;        
    }
  }
}

//------------------------------------TAREFAS VMC------------------------------------------

void verifica_valor_inserido()
{
  if ( valor_inserido > 0 )
  {   
    Serial.print("Valor inserido: ");
    Serial.print(valor_inserido);
    Serial.println(".");
      
    
    switch(valor_inserido)
    {
      /*case 500:       
              receita_total = receita_total + 5;
              escreve_eeprom(EEPROM_ADDR_RECEITA_TOTAL_1, EEPROM_ADDR_RECEITA_TOTAL_2, receita_total);
              i_receita_total = i_receita_total + 5;
              escreve_eeprom(EEPROM_ADDR_I_RECEITA_TOTAL_1, EEPROM_ADDR_I_RECEITA_TOTAL_2, i_receita_total);       
              qtd_moedas_dispensar = 1;
              timeout_motor = 21000;
              valor_inserido = 0;   
              controle_vmc = 1;     
              ultimo_valor_inserido = 500;      
              break;*/
      case 1000:
              receita_total = receita_total + 10;
              escreve_eeprom(EEPROM_ADDR_RECEITA_TOTAL_1, EEPROM_ADDR_RECEITA_TOTAL_2, receita_total);
              i_receita_total = i_receita_total + 10;
              escreve_eeprom(EEPROM_ADDR_I_RECEITA_TOTAL_1, EEPROM_ADDR_I_RECEITA_TOTAL_2, i_receita_total);  
              qtd_moedas_dispensar = 1;
              timeout_motor = 21000;
              //timeout_motor = 42000;
              valor_inserido = 0;
              controle_vmc = 6;  
              ultimo_valor_inserido = 1000;     
              break;
      case 2000:
              receita_total = receita_total + 20;
              escreve_eeprom(EEPROM_ADDR_RECEITA_TOTAL_1, EEPROM_ADDR_RECEITA_TOTAL_2, receita_total);
              i_receita_total = i_receita_total + 20;
              escreve_eeprom(EEPROM_ADDR_I_RECEITA_TOTAL_1, EEPROM_ADDR_I_RECEITA_TOTAL_2, i_receita_total); 
              qtd_moedas_dispensar = 2;
              timeout_motor = 42000;
              valor_inserido = 0;  
              controle_vmc = 7;    
              ultimo_valor_inserido = 2000;   
              break;  
      /*default:
              ultimo_valor_inserido = valor_inserido; 
              valor_inserido = valor_inserido/500;
              
              receita_total = receita_total + valor_inserido*5;
              escreve_eeprom(EEPROM_ADDR_RECEITA_TOTAL_1, EEPROM_ADDR_RECEITA_TOTAL_2, receita_total);
              
              i_receita_total = i_receita_total + valor_inserido*5;
              escreve_eeprom(EEPROM_ADDR_I_RECEITA_TOTAL_1, EEPROM_ADDR_I_RECEITA_TOTAL_2, i_receita_total); 
              
              qtd_moedas_dispensar = valor_inserido;
              
              timeout_motor = 84000;
              
              valor_inserido = 0;  
              
              controle_vmc = 90;   
              break; */    
    }
    
    Serial.print("Quantidade de moedas a despejar : ");
    Serial.print(qtd_moedas_dispensar);
    Serial.println(".");
    Serial.println("Motor Ligado, fazendo contagem das moedas.");
    Serial.println("");
        
    em_venda = 1;  
  }
}


void entrega_finalizada()
{
  digitalWrite(RELE_1,LOW);
  digitalWrite(RELE_2,LOW); 
  Serial.println("");
  Serial.println("Entrega Finalizada!"); 
  Serial.print("Moedas entregues : "); 
  Serial.print(contador_moedas);  
  Serial.println(".");
  Serial.println("");
  contador_moedas = 0;
  controle_ldr = 0;  
  leitura_rep = 0; 
  controle_timeout_motor = 0;
  
  if ( !teste_entrega )
  {
    //mdb.recebe_escrow();
    status_compra=0;
  }  
  
  status_compra=0;
  time_start_em_venda = millis();
  controle_em_venda = 1;
}

void task_entrega_moeda()
{
  if ( qtd_moedas_dispensar > 0 )
  {
    switch(qtd_moedas_dispensar)
    {
      case 1:
              if(contador_moedas == qtd_moedas_dispensar)
              {
                entrega_finalizada();
                sensor_queda_infra.finaliza_leitura();
                sensor_queda_infra.set_evento_disponivel(0);   
                
                if(!teste_entrega)
                {               
                  controle_vmc = 4;   
                  em_venda = 0;
                }else{
                        controle_vmc = 58;
                      }                  
              }
              break;
      case 2:
              if(contador_moedas == qtd_moedas_dispensar)
              {
                entrega_finalizada();
                sensor_queda_infra.finaliza_leitura();
                sensor_queda_infra.set_evento_disponivel(0);
                
                if(!teste_entrega)
                {            
                  controle_vmc = 8;     
                  em_venda=0;
                }else{
                        controle_vmc = 58;
                      }
              }
              break;
      case 4:
              if(contador_moedas == qtd_moedas_dispensar)
              {
                entrega_finalizada();
                sensor_queda_infra.finaliza_leitura();
                sensor_queda_infra.set_evento_disponivel(0);
                
                if(!teste_entrega)
                {               
                  controle_vmc = 8;  
                  em_venda=0;     
                }else{
                          controle_vmc = 58;
                        }
              }
              break;      
    }
    status_compra = 1;
  }
}

void ldr_count()
{    
  if(leitura_rep == 0 && controle_ldr == 1)
  {    
    int ldr = digitalRead(LDR);
    
  //  if(ldr_max < ldr)
   //   ldr_max = ldr;
    
    Serial.println(ldr);
    //if(ldr<500)
    if( ldr )
    {    
      contador_moedas++;  
      if( !teste_entrega )
      {      
        if(estoque != 0)
        {
          estoque--;
          escreve_eeprom(1001,1002,estoque);
          valor_total_inserido = valor_total_inserido + 1;
          escreve_eeprom(EEPROM_ADDR_VALOR_TOTAL_1, EEPROM_ADDR_VALOR_TOTAL_2, valor_total_inserido);
          i_valor_total_inserido = i_valor_total_inserido + 1;
          escreve_eeprom(EEPROM_ADDR_I_VALOR_TOTAL_1, EEPROM_ADDR_I_VALOR_TOTAL_2, i_valor_total_inserido);
        }
      } 
     // ldr_max = 0;
      
      switch(contador_moedas)
      {
        case 1:
               Serial.println("Primeira moeda.");
               break;
        case 2:
               Serial.println("Segunda moeda.");
               break;
        case 3:
               Serial.println("Terceira moeda.");
               break;
        case 4:
               Serial.println("Quarta moeda.");
               break;       
      }
      
      time_start_ldr = millis();
      leitura_rep = 1;
    }    
  }
  
  if(leitura_rep)
  {    
    tempo_atual_ldr = millis();
    if((tempo_atual_ldr-time_start_ldr) > 500)
    {
       leitura_rep = 0;
    }
  } 
}

void timeout_entrega_moeda()
{
  if(controle_timeout_motor)
  {
    tempo_atual_timeout_motor = millis();
    if((tempo_atual_timeout_motor-time_start_timeout_motor) > timeout_motor)
    {
       controle_timeout_motor = 0;
       status_maquina = 0;
       Serial.println("Timeout, erro.");       
       digitalWrite(RELE_1,LOW);
       digitalWrite(RELE_2,LOW);                
       Serial.println("");
       
       if( controle_dez_eventos < 10 )
       {
         controle_dez_eventos++;
       }
       if( qtd_eventos_falha < 10 )
       {
         qtd_eventos_falha++;
       }else{
              qtd_eventos_falha = 1;
            }
       
       Serial.print("QTD EVENTOS FALHA: ");
       Serial.println(qtd_eventos_falha);
       
       Serial.print("CONTROLE DEZ EVENTOS: ");
       Serial.println(controle_dez_eventos);
       
       escreve_eeprom(1011,1012,qtd_eventos_falha);
       escreve_eeprom(800,801,controle_dez_eventos);
       
       info_falha.data = rtc.now();
       
       Serial.print("Horario: ");
       Serial.println(info_falha.data.unixtime());
       
       pin_parte[0] = (info_falha.data.unixtime())/256;
       pin_parte[1] = (info_falha.data.unixtime())%256;
       pin_parte[2] = pin_parte[0]/256;
       pin_parte[3] = pin_parte[0]%256;
       pin_parte[4] = pin_parte[2]/256;
       pin_parte[5] = pin_parte[2]%256;
       EEPROM.write((qtd_eventos_falha*8)+1013, pin_parte[1]);   
       EEPROM.write((qtd_eventos_falha*8)+1014, pin_parte[3]); 
       EEPROM.write((qtd_eventos_falha*8)+1015, pin_parte[4]);   
       EEPROM.write((qtd_eventos_falha*8)+1016, pin_parte[5]); 
       
       Serial.print("Depois de salvar Horario: ");
       Serial.println((((((pin_parte[4]*256)+pin_parte[5])*256)+pin_parte[3])*256)+pin_parte[1]);
       
       info_falha.valor_depositado = ultimo_valor_inserido;
       escreve_eeprom((qtd_eventos_falha*8)+1017,(qtd_eventos_falha*8)+1018,info_falha.valor_depositado);
       Serial.print("VALOR DEPOSITADO: ");
       Serial.println(info_falha.valor_depositado);
       
       info_falha.moedas_disp = contador_moedas;
       escreve_eeprom((qtd_eventos_falha*8)+1019,(qtd_eventos_falha*8)+1020,info_falha.moedas_disp);
       Serial.print("MOEDAS DIPENSADAS: ");
       Serial.println(info_falha.moedas_disp);
       
       
       contador_moedas = 0;
       controle_ldr = 0;  
       leitura_rep = 0; 
       valor_inserido_bill_escrow = 0;
       valor_inserido_bill = 0;
       if(!teste_entrega)
       {
          controle_bill=0;
          boot_mdb=0;
          controle_vmc = 10;
          status_compra=0;
       }else
       {
         controle_vmc = 60;
       }
       //mdb.rejeita_escrow();
       mdb.desabilita_bill();
       status_compra = 0;
    }
  }
}

void verifica_mudanca_status()
{
   if(aux != status_vmc)
   {
     controle_vmc = 0;
     aux = status_vmc;
   }
}

//Tarefa que ativa o buzzer a cada botao acionado.
void buzzer_atv()
{    
  //Quando controle_buzzer igual a 1.
  if(controle_buzzer)
  {    
    controle_buzzer = 2;
    //Liga o pino do buzzer.
    time_start_retorna = millis();
    digitalWrite(BUZZER,HIGH);    
  }
  //Quando controle_buzzer igual a 2.
  if(controle_buzzer==2)
  {
    //Aguarda o tempo de DELAY_BUZZER para desligar o buzzer.
    tempo_atual_buzzer= millis();
    if((tempo_atual_buzzer-time_start_buzzer) > DELAY_BUZZER)
    {
        //Desliga pino do buzzer.
        digitalWrite(BUZZER,LOW);
        controle_buzzer=0;
    } 
  }  
}

void inatividade()
{
  if(controle_vmc>20)
  {
     tempo_atual_retorna= millis();
     if((tempo_atual_retorna-time_start_retorna) > 35000)
      {
          lcd.noBlink();
          controle_vmc = 0;
      }
  }
}

void verifica_estoque()
{
  if ( estoque < 5 )
  {
    mdb.desabilita_bill();
  }
}

// Software reset usando watchdog timer (método seguro)
// Reinicia todos os registradores e periféricos corretamente
void softReset()
{
  wdt_enable(WDTO_15MS);  // Ativa watchdog com timeout de 15ms
  while(1) {}             // Aguarda o watchdog resetar o sistema
}

//Tarefa que faz a leitura do horario salvo e mostra na tela principal.
void printDate()
{  
  // Obtem a data e hora correntes e armazena em tstamp
  DateTime tstamp = rtc.now();  
 
  if((tstamp.year()-2000)==165)
  {
  }else{
    //Mostra na tela hora e data.
    if(tstamp.hour()<10)                                      
    {
      lcd.setCursor(2,2);
      lcd.print(F("0"));
      lcd.setCursor(3,2);
      lcd.print(tstamp.hour());
    }else{
            lcd.setCursor(2,2);
            lcd.print(tstamp.hour());
          }       
    //lcd.setCursor(4,2);
    //lcd.print(F(":"));
    if(tstamp.minute()<10)
    {
      lcd.setCursor(5,2);
      lcd.print(F("0"));
      lcd.setCursor(6,2);
      lcd.print(tstamp.minute());
      
    }else{
            lcd.setCursor(5,2);
            lcd.print(tstamp.minute());
          }
    lcd.setCursor(8,2);
    lcd.print(F("/")); 
    if(tstamp.day()<10)
    {
      lcd.setCursor(10,2);
      lcd.print(F("0"));
      lcd.setCursor(11,2);
      lcd.print(tstamp.day());
      
    }else{
            lcd.setCursor(10,2);
            lcd.print(tstamp.day());
          }    
    lcd.print(F("/"));
    if(tstamp.month()<10)
    {
      lcd.setCursor(13,2);
      lcd.print(F("0"));
      lcd.setCursor(14,2);
      lcd.print(tstamp.month());
    }else{
            lcd.setCursor(13,2);
            lcd.print(tstamp.month());
          }
    lcd.setCursor(15,2);
    lcd.print(F("/"));
   if((tstamp.year()-2000)<10)
    {
      lcd.setCursor(16,2);
      lcd.print(F("0"));
      lcd.setCursor(17,2);
      lcd.print((tstamp.year()-2000));
    }else
    {
       lcd.setCursor(16,2);
        lcd.print((tstamp.year()-2000));
    }
  }
}

//Funcao para visualizar o horario e data no relatorio de vendas.
void printDate_relatorio_falha(int tipo, DateTime tstamp)
{
   if ( tipo == 3 ){
    //Mostra na tela hora e data.
      if(tstamp.hour()<10)                                      
      {
        lcd2.setCursor(2,3);
        lcd2.print(F("0"));
        lcd2.setCursor(3,3);
        lcd2.print(tstamp.hour());
      }else{
              lcd2.setCursor(2,3);
              lcd2.print(tstamp.hour());
            }       
      lcd2.setCursor(4,3);
      lcd2.print(F(":"));
      if(tstamp.minute()<10)
      {
        lcd2.setCursor(5,3);
        lcd2.print(F("0"));
        lcd2.setCursor(6,3);
        lcd2.print(tstamp.minute());
        
      }else{
              lcd2.setCursor(5,3);
              lcd2.print(tstamp.minute());
            }
      lcd2.setCursor(8,3);
      lcd2.print(F("/")); 
      if(tstamp.day()<10)
      {
        lcd2.setCursor(10,3);
        lcd2.print(F("0"));
        lcd2.setCursor(11,3);
        lcd2.print(tstamp.day());
        
      }else{
              lcd2.setCursor(10,3);
              lcd2.print(tstamp.day());
            } 
       lcd2.setCursor(12,3);     
       lcd2.print(F("/"));     
      if(tstamp.month()<10)
      {
        lcd2.setCursor(13,3);
        lcd2.print(F("0"));
        lcd2.setCursor(14,3);
        lcd2.print(tstamp.month());
      }else{
              lcd2.setCursor(13,3);
              lcd2.print(tstamp.month());
            }
      lcd2.setCursor(15,3);      
      lcd2.print(F("/"));     
      if((tstamp.year()-2000)<10)
      {  
        lcd2.setCursor(16,3);
        lcd2.print(F("0"));
        lcd2.setCursor(17,3);
        lcd2.print((tstamp.year()-2000));
      }else
      {
         lcd2.setCursor(16,3);
          lcd2.print((tstamp.year()-2000));
      }
  }else{
    //Mostra na tela hora e data.
    if(tstamp.hour()<10)                                      
    {
      lcd2.setCursor(2,2);
      lcd2.print(F("0"));
      lcd2.setCursor(3,2);
      lcd2.print(tstamp.hour());
    }else{
            lcd2.setCursor(2,2);
            lcd2.print(tstamp.hour());
          }       
    lcd2.setCursor(4,2);
    lcd2.print(F(":"));
    if(tstamp.minute()<10)
    {
      lcd2.setCursor(5,2);
      lcd2.print(F("0"));
      lcd2.setCursor(6,2);
      lcd2.print(tstamp.minute());
      
    }else{
            lcd2.setCursor(5,2);
            lcd2.print(tstamp.minute());
          }
    lcd2.setCursor(8,2);
    lcd2.print(F("/")); 
    if(tstamp.day()<10)
    {
      lcd2.setCursor(10,2);
      lcd2.print(F("0"));
      lcd2.setCursor(11,2);
      lcd2.print(tstamp.day());
      
    }else{
            lcd2.setCursor(10,2);
            lcd2.print(tstamp.day());
          }    
    lcd2.print(F("/"));
    if(tstamp.month()<10)
    {
      lcd2.setCursor(13,2);
      lcd2.print(F("0"));
      lcd2.setCursor(14,2);
      lcd2.print(tstamp.month());
    }else{
            lcd2.setCursor(13,2);
            lcd2.print(tstamp.month());
          }
    lcd2.setCursor(15,2);
    lcd2.print(F("/"));
   if((tstamp.year()-2000)<10)
    {
      lcd2.setCursor(16,2);
      lcd2.print(F("0"));
      lcd2.setCursor(17,2);
      lcd2.print((tstamp.year()-2000));
    }else
    {
       lcd2.setCursor(16,2);
        lcd2.print((tstamp.year()-2000));
    }
  } 
}

//Funcao para visualizar o horario e data no relatorio de vendas.
void printDate_relatorio(int tipo)
{ 
  // Obtem a data e hora correntes e armazena em tstamp
  DateTime tstamp = rtc.now();
  
  //Vendas ATIVAS.
  if(tipo==1)
  {
    if((tstamp.year()-2000)==165)
    {
    }else{
      //Mostra na tela hora e data.
      if(tstamp.hour()<10)                                      
      {
        lcd2.setCursor(2,0);
        lcd2.print(F("0"));
        lcd2.setCursor(3,0);
        lcd2.print(tstamp.hour());
      }else{
              lcd2.setCursor(2,0);
              lcd2.print(tstamp.hour());
            }       
      lcd2.setCursor(4,0);
      lcd2.print(F(":"));
      if(tstamp.minute()<10)
      {
        lcd2.setCursor(5,0);
        lcd2.print(F("0"));
        lcd2.setCursor(6,0);
        lcd2.print(tstamp.minute());
        
      }else{
              lcd2.setCursor(5,0);
              lcd2.print(tstamp.minute());
            }
      lcd2.setCursor(8,0);
      lcd2.print(F("/")); 
      if(tstamp.day()<10)
      {
        lcd2.setCursor(10,0);
        lcd2.print(F("0"));
        lcd2.setCursor(11,0);
        lcd2.print(tstamp.day());
        
      }else{
              lcd2.setCursor(10,0);
              lcd2.print(tstamp.day());
            } 
       lcd2.setCursor(12,0);     
       lcd2.print(F("/"));     
      if(tstamp.month()<10)
      {
        lcd2.setCursor(13,0);
        lcd2.print(F("0"));
        lcd2.setCursor(14,0);
        lcd2.print(tstamp.month());
      }else{
              lcd2.setCursor(13,0);
              lcd2.print(tstamp.month());
            }
      lcd2.setCursor(15,0);      
      lcd2.print(F("/"));     
      if((tstamp.year()-2000)<10)
      {  
        lcd2.setCursor(16,0);
        lcd2.print(F("0"));
        lcd2.setCursor(17,0);
        lcd2.print((tstamp.year()-2000));
      }else
      {
         lcd2.setCursor(16,0);
          lcd2.print((tstamp.year()-2000));
      }    
    }
  }else if(tipo==2){
    //Venda ININTERRUPTA
      lcd2.setCursor(17,0);
      lcd2.print(F("FX"));
     if((tstamp.year()-2000)==165)
    {
    }else{
      //Mostra na tela hora e data.
      if(tstamp.hour()<10)                                      
      {
        lcd2.setCursor(0,0);
        lcd2.print(F("0"));
        lcd2.setCursor(1,0);
        lcd2.print(tstamp.hour());
      }else{
              lcd2.setCursor(0,0);
              lcd2.print(tstamp.hour());
            }       
      lcd2.setCursor(2,0);
      lcd2.print(F(":"));
      if(tstamp.minute()<10)
      {
        lcd2.setCursor(3,0);
        lcd2.print(F("0"));
        lcd2.setCursor(4,0);
        lcd2.print(tstamp.minute());
        
      }else{
              lcd2.setCursor(3,0);
              lcd2.print(tstamp.minute());
            }
      lcd2.setCursor(6,0);
      lcd2.print(F("/")); 
      if(tstamp.day()<10)
      {
        lcd2.setCursor(8,0);
        lcd2.print(F("0"));
        lcd2.setCursor(9,0);
        lcd2.print(tstamp.day());
        
      }else{
              lcd2.setCursor(8,0);
              lcd2.print(tstamp.day());
            }    
      lcd2.setCursor(10,0);      
      lcd2.print(F("/"));
      if(tstamp.month()<10)
      {
        lcd2.setCursor(11,0);
        lcd2.print(F("0"));
        lcd2.setCursor(12,0);
        lcd2.print(tstamp.month());
      }else{
              lcd2.setCursor(11,0);
              lcd2.print(tstamp.month());
            }
      lcd2.setCursor(13,0);      
      lcd2.print(F("/"));      
      if((tstamp.year()-2000)<10)
      {  
        lcd2.setCursor(14,0);
        lcd2.print(F("0"));
        lcd2.setCursor(15,0);
        lcd2.print((tstamp.year()-2000));
      }else
      {
         lcd2.setCursor(14,0);
          lcd2.print((tstamp.year()-2000));
      }    
    }
  }
}

void pisca_pino()
{
 if((tempo_atual_pisca-time_start_pisca) > 500)
   {
       if(digitalRead(13)==HIGH)
      {
        digitalWrite(13,LOW);
      }else{
              digitalWrite(13,HIGH);
            }
       time_start_pisca = millis();         
   }  
}

//Mostra o horario e a data na tela inicial do sistema.
void mostra_data()
{  
  if( (controle_vmc == 0 || controle_vmc == 20) && !em_venda && inicializacao_ok==1)
  {
    // Faz piscar os : do horario a cada 1 segundo.
    tempo_atual_piscap = millis();
    if((tempo_atual_piscap-time_start_piscap) > 1000 && estoque > 4 && status_vmc)
    {
       if ( pisca_pontos == 0 )
       {
         pisca_pontos = 1;
         lcd.setCursor(4,2);
         lcd.print(F(" "));
       }else
       {
         pisca_pontos = 0;
         lcd.setCursor(4,2);
         lcd.print(F(":"));
       }
       time_start_piscap = millis();     
    }
    tempo_atual_lcd= millis();
    if((tempo_atual_lcd-time_start_lcd) > 10000)
     {
         reinicia_lcd();          
         controle_vmc = 0;
         status_compra= 0;
         time_start_lcd = tempo_atual_lcd;         
     }      
    lcd.noBlink();
    //Funcao que le os dados do 12c.
    if(status_vmc && estoque>4)
    {
      printDate();
    }    
  }
}

void reseta_sistema()
{
  DateTime tstamp = rtc.now();
  
  if(tstamp.hour()==2 && tstamp.minute()==0 && tstamp.second()==0)
  {
    delay(1000);
    softReset();
  }
}

void inibi_notas()
{
  if(estoque>=4)
  {
      mdb.inicia_notas(1,1,1);
      mdb.habilita_bill();
  }
  if(estoque<2)
  {
    if(altera_estado_notas!=2)
    {
      mdb.inicia_notas(1,0,0);
      mdb.habilita_bill();
      altera_estado_notas = 2;
    }
  }
  if(estoque<4)
  {
    if(altera_estado_notas!=3)
    {
      mdb.inicia_notas(1,1,0);
      mdb.habilita_bill();
      altera_estado_notas = 3;
    }
  }
}

void verifica_em_venda()
{
  if(controle_em_venda)
  {
    tempo_atual_em_venda= millis();
    if((tempo_atual_em_venda-time_start_em_venda) > 5000)
     {
       if(valor_inserido>0)
       {
         em_venda = 0;
       }
       controle_em_venda = 0;
     }
  }
}


/*********************************************************************************************************
** Nome da Função:       mdb_deposito
** Descrição:            Tarefa responsavel por verificar se o mdb indicou um valor depositado.
** Parametro:            Não.
** Valor de retorno:     Não.
*********************************************************************************************************/
void mdb_deposito()
{
  // Verifica se existe um deposito realizado pelo MDB.
  if ( (mdb.get_valor_depositado()) != 0 && !em_venda )
  { 
    Serial.print("VALOR DEPOSITADO: ");
    Serial.println(mdb.get_valor_depositado());
    Serial.print("ESTADO ACEITACAO: ");
    Serial.println(mdb.get_estado_aceitacao());
    
    // De acordo com o estado da aceitação(exemplo: moeda foi para o tubo ou moeda foi para o cofre).
    switch(mdb.get_estado_aceitacao())
    {
      case BILL_STACKED:
            Serial.println("NOTA NO STACKED.");
            valor_inserido = mdb.get_valor_depositado();
            controle = 1;
            break;
      case ESCROW_POSITION:
            Serial.println("NOTA EM CUSTODIA.");
            valor_inserido = mdb.get_valor_depositado();  
            controle = 1;  
            break;
      case ESCROW_STACKED:
            Serial.println("NOTA DA CUSTODIA PARA STACKED.");
            //valor_inserido = mdb.get_valor_depositado();
            break;            
    }
    
    // Seta o valor depositado em 0.
    mdb.set_valor_depositado(0);
    
  }
}

/*********************************************************************************************************
** Nome da Função:       mdb_task_main
** Descrição:            Tarefa responsavel por verificar as ações vindas do mdb.
** Parametro:            Não.
** Valor de retorno:     Não.
*********************************************************************************************************/
void mdb_task_main()
{    
  // Tarefa responsavel por verificar se o mdb indicou um valor depositado.
  mdb_deposito();
    
  // Método da classe que é a rotina de leitura dos componentes e poller.
  mdb.task();
}

void visualiza_evento_falha()
{
  lcd2.setCursor(0,1);            
  lcd2.print(F("      EVENTO      "));
  lcd2.setCursor(13,1);   
  lcd2.print(posicao+1);
  pin_parte[1] = EEPROM.read(((posicao+1)*8)+1013);
  pin_parte[3] = EEPROM.read(((posicao+1)*8)+1014);
  pin_parte[4] = EEPROM.read(((posicao+1)*8)+1015);
  pin_parte[5] = EEPROM.read(((posicao+1)*8)+1016);
  info_falha.data = (((((pin_parte[4]*256)+pin_parte[5])*256)+pin_parte[3])*256)+pin_parte[1];
  Serial.print("Horario: ");
  Serial.println(info_falha.data.unixtime());
  lcd2.setCursor(0,2);            
  lcd2.print(F("                  "));
  printDate_relatorio_falha(4, info_falha.data);
}

void detail_evento_falha()
{
  Serial.println("QTD EVENTOS FALHA: ");
  Serial.print(qtd_eventos_falha);
  
  Serial.println("POSICAO: ");
  Serial.print(posicao);
  
  lcd2.clear();
  lcd2.setCursor(0,0);            
  lcd2.print(F("QTD MOEDAS DISPENS.:"));
  info_falha.moedas_disp = read_eeprom(((posicao+1)*8)+1019,((posicao+1)*8)+1020);
  
  lcd2.setCursor(0,1);            
  lcd2.print(F("        00"));
  lcd2.setCursor(10,1);  
  lcd2.print(info_falha.moedas_disp);
  Serial.print("MOEDAS DIPENSADAS: ");
  Serial.println(info_falha.moedas_disp);
  
  lcd2.setCursor(0,2);            
  lcd2.print(F("VALOR INSERIDO: "));
  info_falha.valor_depositado = read_eeprom(((posicao+1)*8)+1017,((posicao+1)*8)+1018);
  lcd2.setCursor(15,2);
  lcd2.print(info_falha.valor_depositado);
  Serial.print("VALOR DEPOSITADO: ");
  Serial.println(info_falha.valor_depositado);
  
  lcd2.setCursor(0,3);            
  lcd2.print(F("                  "));
  printDate_relatorio_falha(3, info_falha.data);
}

void task_controladora()
{
  switch(controle)
  {
    case 1:
          switch(valor_inserido)
          {
            /*case 500:       
                    lcd.clear();
                    lcd.setCursor(0,0);
                    lcd.print(F("     AGUARDE..."));
                    lcd.setCursor(0,1);
                    lcd.print(F(" VALOR INS.: R$5,00"));
                    lcd.setCursor(0,2);
                    lcd.print(F("   QTD MOEDAS: 1."));  
                    lcd.setCursor(0,3);
                    lcd.print(F("  DISPENSADAS: 0."));  
                    receita_total = receita_total + 5;
                    escreve_eeprom(EEPROM_ADDR_RECEITA_TOTAL_1, EEPROM_ADDR_RECEITA_TOTAL_2, receita_total);
                    i_receita_total = i_receita_total + 5;
                    escreve_eeprom(EEPROM_ADDR_I_RECEITA_TOTAL_1, EEPROM_ADDR_I_RECEITA_TOTAL_2, i_receita_total);       
                    qtd_moedas_dispensar = 1;
                    timeout_motor = 21000;
                    valor_inserido = 0;       
                    ultimo_valor_inserido = 500;   
                    break;*/
            case 1000:
                    lcd.clear();
                    lcd.setCursor(0,0);
                    lcd.print(F("     AGUARDE..."));
                    lcd.setCursor(0,1);
                    lcd.print(F("VALOR INS.: R$10,00"));
                    lcd.setCursor(0,2);
                    lcd.print(F("   QTD MOEDAS: 1."));  
                    lcd.setCursor(0,3);
                    lcd.print(F("  DISPENSADAS: 0."));  
                    receita_total = receita_total + 10;
                    escreve_eeprom(EEPROM_ADDR_RECEITA_TOTAL_1, EEPROM_ADDR_RECEITA_TOTAL_2, receita_total);
                    i_receita_total = i_receita_total + 10;
                    escreve_eeprom(EEPROM_ADDR_I_RECEITA_TOTAL_1, EEPROM_ADDR_I_RECEITA_TOTAL_2, i_receita_total);  
                    qtd_moedas_dispensar = 1;
                    timeout_motor = 21000;
                    valor_inserido = 0;
                    ultimo_valor_inserido = 1000;     
                    break;
            case 2000:
                    lcd.clear();
                    lcd.setCursor(0,0);
                    lcd.print(F("     AGUARDE..."));
                    lcd.setCursor(0,1);
                    lcd.print(F("VALOR INS.: R$20,00"));
                    lcd.setCursor(0,2);
                    lcd.print(F("   QTD MOEDAS: 2.")); 
                    lcd.setCursor(0,3);
                    lcd.print(F("  DISPENSADAS: 0."));   
                    receita_total = receita_total + 20;
                    escreve_eeprom(EEPROM_ADDR_RECEITA_TOTAL_1, EEPROM_ADDR_RECEITA_TOTAL_2, receita_total);
                    i_receita_total = i_receita_total + 20;
                    escreve_eeprom(EEPROM_ADDR_I_RECEITA_TOTAL_1, EEPROM_ADDR_I_RECEITA_TOTAL_2, i_receita_total); 
                    qtd_moedas_dispensar = 2;
                    timeout_motor = 42000;
                    valor_inserido = 0;  
                    ultimo_valor_inserido = 2000;   
                    break;
          }          
          time_start_value = millis(); 
          em_venda = 1; 
          controle++;
          break;
     case 2:
          tempo_atual_value = millis();
          if((tempo_atual_value-time_start_value) > 500)
          {
            time_start_value = tempo_atual_value; 
            digitalWrite(RELE_2,HIGH);
            sensor_queda_infra.realiza_leitura();
            sensor_queda_infra.set_evento_disponivel(0);   
            contador_moedas = 0;
            time_start_timeout_motor = millis();   
            time_start_infra = millis();	  
            limpa_registro = 0;        
            controle++;
          } 
          break;
     case 3:       
          tempo_atual_timeout_motor = millis();
          if((tempo_atual_timeout_motor-time_start_timeout_motor) > timeout_motor)
          {
             controle_timeout_motor = 0;
             status_maquina = 0;
             Serial.println("Timeout, erro.");          
             Serial.println("");
             digitalWrite(RELE_2,LOW);       
             
             if( controle_dez_eventos < 10 )
             {
               controle_dez_eventos++;
             }
             if( qtd_eventos_falha < 10 )
             {
               qtd_eventos_falha++;
             }else{
                    qtd_eventos_falha = 1;
                  }
             
             Serial.print("QTD EVENTOS FALHA: ");
             Serial.println(qtd_eventos_falha);
             
             Serial.print("CONTROLE DEZ EVENTOS: ");
             Serial.println(controle_dez_eventos);
             
             escreve_eeprom(1011,1012,qtd_eventos_falha);
             escreve_eeprom(800,801,controle_dez_eventos);
             
             info_falha.data = rtc.now();
             
             Serial.print("Horario: ");
             Serial.println(info_falha.data.unixtime());
             
             pin_parte[0] = (info_falha.data.unixtime())/256;
             pin_parte[1] = (info_falha.data.unixtime())%256;
             pin_parte[2] = pin_parte[0]/256;
             pin_parte[3] = pin_parte[0]%256;
             pin_parte[4] = pin_parte[2]/256;
             pin_parte[5] = pin_parte[2]%256;
             EEPROM.write((qtd_eventos_falha*8)+1013, pin_parte[1]);   
             EEPROM.write((qtd_eventos_falha*8)+1014, pin_parte[3]); 
             EEPROM.write((qtd_eventos_falha*8)+1015, pin_parte[4]);   
             EEPROM.write((qtd_eventos_falha*8)+1016, pin_parte[5]); 
             
             Serial.print("Depois de salvar Horario: ");
             Serial.println((((((pin_parte[4]*256)+pin_parte[5])*256)+pin_parte[3])*256)+pin_parte[1]);
             
             info_falha.valor_depositado = ultimo_valor_inserido;
             escreve_eeprom((qtd_eventos_falha*8)+1017,(qtd_eventos_falha*8)+1018,info_falha.valor_depositado);
             Serial.print("VALOR DEPOSITADO: ");
             Serial.println(info_falha.valor_depositado);
             
             info_falha.moedas_disp = contador_moedas;
             escreve_eeprom((qtd_eventos_falha*8)+1019,(qtd_eventos_falha*8)+1020,info_falha.moedas_disp);
             Serial.print("MOEDAS DIPENSADAS: ");
             Serial.println(info_falha.moedas_disp);
             
             
             contador_moedas = 0;
             controle_ldr = 0;  
             leitura_rep = 0; 
             valor_inserido_bill_escrow = 0;
             valor_inserido_bill = 0;
             controle_bill = 0;
             boot_mdb = 0;
             controle = 6;
             status_compra = 0;
             //mdb.rejeita_escrow();
             mdb.desabilita_bill();
             status_compra = 0;
          }   
          // Verifica Timeout de 15 segundos caso não constate verificação do sensor da base.
          tempo_atual_infra = millis(); 
          if((tempo_atual_infra-time_start_infra) > 200)
          {  
            if ( limpa_registro )
            {
              sensor_queda_infra.set_evento_disponivel(0);  
              limpa_registro = 0;
            }
            if ( sensor_queda_infra.get_evento_disponivel() )
            {             
               Serial.print("MOEDA DISPENSADA. Canal Leitura: ");       
               Serial.println(sensor_queda_infra.get_canal_detectado());
               contador_moedas++;       
               lcd.setCursor(15,3);
               lcd.print(contador_moedas);  
               if(estoque != 0)
               {
                 estoque--;
                 escreve_eeprom(1001,1002,estoque);
                 valor_total_inserido = valor_total_inserido + 1;
                 escreve_eeprom(EEPROM_ADDR_VALOR_TOTAL_1, EEPROM_ADDR_VALOR_TOTAL_2, valor_total_inserido);
                 i_valor_total_inserido = i_valor_total_inserido + 1;
                 escreve_eeprom(EEPROM_ADDR_I_VALOR_TOTAL_1, EEPROM_ADDR_I_VALOR_TOTAL_2, i_valor_total_inserido);
               }
               //sensor_queda_infra.finaliza_leitura();     
               sensor_queda_infra.set_evento_disponivel(0);   
               time_start_infra = millis();	
               limpa_registro = 1;
            }    
          }
          if(contador_moedas == qtd_moedas_dispensar)
          {
            digitalWrite(RELE_2,LOW);   
            entrega_finalizada();
            sensor_queda_infra.finaliza_leitura();
            sensor_queda_infra.set_evento_disponivel(0);   
            
            controle++;                          
          }       
          break;
     case 4:          
          lcd.clear();
          lcd.setCursor(0,0);
          if ( qtd_moedas_dispensar > 1 )
            lcd.print(F("MOEDA ENTREGUE"));
          else
            lcd.print(F("MOEDAS ENTREGUES"));  
          lcd.setCursor(0,1);
          lcd.print(F("COM SUCESSO,"));
          lcd.setCursor(0,2);
          lcd.print(F("RETIRE NA SAIDA"));
          lcd.setCursor(0,3);
          lcd.print(F("INDICADA, OBRIGADO!"));
          time_start_value = millis();   
          controle++;
          break;
     case 5:
          tempo_atual_value = millis();
          if((tempo_atual_value-time_start_value) > 3500)
          {
            time_start_value = tempo_atual_value;             
            em_venda = 0;  
            controle = 0;
          } 
          break;
     case 6:
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print(F("ERRO AO ENTREGAR"));
            lcd.setCursor(0,1);
            lcd.print(F("MOEDAS, MAQUINA")); 
            lcd.setCursor(0,2);
            lcd.print(F("FICARA INOPERANTE."));    
            time_start_value = millis();   
            controle++;
            break;
     case 7:
            tempo_atual_value = millis();
            if((tempo_atual_value-time_start_value) > 3500)
            {
              controle++;
            }      
            break;
     case 8:
            lcd.clear();
            lcd.setCursor(0,1);
            lcd.print(F("   EQUIPAMENTO EM"));
            lcd.setCursor(0,2);
            lcd.print(F("     MANUTENCAO.")); 
            time_start_value = millis();   
            controle++;
            break;
     case 9:
            tempo_atual_value = millis();
            if((tempo_atual_value-time_start_value) > 3500)
            {
              time_start_value = tempo_atual_value;    
              status_vmc = 0;
              aux = status_vmc;
              EEPROM.write(EEPROM_ADDR_STATUS_VMC, status_vmc);
              controle = 0;
            }      
            break;         
  }
}

void loop() 
{
   // Tarefa de comunicação com o sensor de queda infravermelho.
   sensor_queda_infra.task();
      
   task_controladora();   
   
   mostra_data();
   
   aguarda_inicializacao();   
   
   if ( inicializacao_ok && status_vmc )   
     mdb_task_main();
   
   statemachine_vmc();
   
   inatividade();
   
   buzzer_atv();
   
   verifica_estoque();
  
  if(Serial.available())
  {
      int msg = Serial.read();
      
      switch(msg)
      {
         case '0':
                 status_vmc = 1;
                 break; 
        case '1':
                valor_inserido = 1000;
                break; 
        case '2': 
                valor_inserido = 500;
                break;
        case '3':
               sensor_queda_infra.realiza_leitura();
                sensor_queda_infra.set_evento_disponivel(0);  
               /*qtd_eventos_falha = 0;
               escreve_eeprom(1011,1012,qtd_eventos_falha);
               controle_dez_eventos = 0;  
               escreve_eeprom(800,801,controle_dez_eventos);*/
               break;  
        case '4':
                sensor_queda_infra.finaliza_leitura();
               /*qtd_eventos_falha = 10;
               escreve_eeprom(1011,1012,qtd_eventos_falha);
               controle_dez_eventos = 10;  
               escreve_eeprom(800,801,controle_dez_eventos);*/
                break;  
        case '5':
                first_time = 20;
                EEPROM.write(EEPROM_ADDR_FIRST_TIME, first_time);
                break;
       case '6':
                mdb.inicia_notas(1,1,1);
                mdb.habilita_bill();
                break; 
       case '7':
                escreve_eeprom(EEPROM_ADDR_VALOR_TOTAL_1, EEPROM_ADDR_VALOR_TOTAL_2, 0);
                escreve_eeprom(EEPROM_ADDR_I_VALOR_TOTAL_1, EEPROM_ADDR_I_VALOR_TOTAL_2, 0);
                escreve_eeprom(EEPROM_ADDR_RECEITA_TOTAL_1, EEPROM_ADDR_RECEITA_TOTAL_2, 0);
                escreve_eeprom(EEPROM_ADDR_I_RECEITA_TOTAL_1, EEPROM_ADDR_I_RECEITA_TOTAL_2, 0);   
                break; 
       case '8':
                Serial.print("Valor inserido: ");
                Serial.println(valor_inserido);
                Serial.print("Em venda : ");
                Serial.println(em_venda);
                break;          
                
      }
  }
}
