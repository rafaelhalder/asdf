/****************************************Copyright (c)*************************************************
**                      Power Vending - A. R. M. Venda Automatica Ltda.
**                             http://www.powervending.com.br
**--------------Informação do Arquivo -----------------------------------------------------------------
** Nome do Arquivo:          VMCState.h
** Data Ultima Modificação:  20-11-24
** Ultima Versão:            Sim
** Descrição:                Estruturas para organização de variáveis de estado
**                           Parte do plano de refatoração para reduzir variáveis globais
**------------------------------------------------------------------------------------------------------
** Criado por:          Rafael Henrique (rafaelhalder@gmail.com)
** Data de Criação:     20-11-24
********************************************************************************************************/

#ifndef VMCState_h
#define VMCState_h

#include <Arduino.h>

// Estruturas para organização de variáveis de estado
// Reduz 97+ variáveis globais para grupos lógicos
// Migração gradual em fases

// VMC - Vending Machine Controller
struct VMCState {
  int controle;                    // Controle geral de estado
  int controle_vmc;                // Estado da FSM de venda
  int valor_inserido;              // Valor inserido (centavos)
  int valor_inserido_total;        // Total moedas+notas
  bool em_venda;                   // Venda em andamento
  bool controle_em_venda;          // Controle auxiliar venda
  bool status_compra;              // Status compra atual
  bool status_vmc;                 // Status VMC
  int aux;                         // Variável auxiliar
  int posicao;                     // Produto selecionado
  
  // Construtor com valores padrão
  VMCState() : 
    controle(0),
    controle_vmc(0),
    valor_inserido(0),
    valor_inserido_total(0),
    em_venda(false),
    controle_em_venda(false),
    status_compra(false),
    status_vmc(false),
    aux(0),
    posicao(0)
  {}
};

// MDB - Multi-Drop Bus (comunicação moedeiro/noteiro)
struct MDBState {
  bool boot_mdb;                   // MDB completou boot
  bool inicializacao_ok;           // Inicialização OK
  int controle_bill;               // Estado FSM bill
  int contador_bill;               // Contador bill
  int escrow_ativo;                // Escrow ativo
  int valor_inserido_bill_escrow;  // Valor nota em escrow
  int valor_inserido_bill;         // Total notas
  int controle_deposito_mdb;       // Controle depósito
  int mdb_task_ctl;                // Controle task MDB
  int lendo_bill;                  // Lendo nota
  bool mdb_bill_sem_atividade;     // Bill sem resposta
  bool aguarda_reset_bill;         // Aguarda reset bill
  bool controle_ack;               // Controle ACK
  int type_escrow_1;               // Tipo escrow byte 1
  int type_escrow_2;               // Tipo escrow byte 2
  int bill_type_deposited[5];      // Tipo nota depositada
  int bill_routing[3];             // Roteamento nota
  int dado_poll[10];               // Dados poll MDB
  
  // Construtor com valores padrão
  MDBState() :
    boot_mdb(false),
    inicializacao_ok(false),
    controle_bill(0),
    contador_bill(0),
    escrow_ativo(0),
    valor_inserido_bill_escrow(0),
    valor_inserido_bill(0),
    controle_deposito_mdb(0),
    mdb_task_ctl(0),
    lendo_bill(0),
    mdb_bill_sem_atividade(false),
    aguarda_reset_bill(false),
    controle_ack(false),
    type_escrow_1(0x00),
    type_escrow_2(0x00)
  {
    memset(bill_type_deposited, 0, sizeof(bill_type_deposited));
    memset(bill_routing, 0, sizeof(bill_routing));
    memset(dado_poll, 0, sizeof(dado_poll));
  }
};

// Vendas e Estoque
struct SalesState {
  long estoque;                    // Quantidade em estoque
  long valor_total_inserido;       // Total inserido (histórico)
  long i_valor_total_inserido;     // Parte inteira do total
  long receita_total;              // Receita total
  long i_receita_total;            // Parte inteira receita
  int ultimo_valor_inserido;       // Último valor (display)
  int qtd_eventos_falha;           // Contador falhas
  int controle_dez_eventos;        // Controle últimos 10 eventos
  
  // Construtor com valores padrão
  SalesState() :
    estoque(0),
    valor_total_inserido(0),
    i_valor_total_inserido(0),
    receita_total(0),
    i_receita_total(0),
    ultimo_valor_inserido(0),
    qtd_eventos_falha(0),
    controle_dez_eventos(0)
  {}
};

// Display e Interface
struct DisplayState {
  bool pisca_pontos;               // Piscar pontos relógio
  int controle_visualiza;          // Controle visualização
  int mostra_msg_ini;              // Mostrar mensagem inicial
  int linha_ini;                   // Linha inicial display
  unsigned short int hora_1, hora_2;
  unsigned short int minuto_1, minuto_2;
  unsigned short int dia_1, dia_2;
  unsigned short int mes_1, mes_2;
  unsigned short int ano_1, ano_2;
  
  // Construtor com valores padrão
  DisplayState() :
    pisca_pontos(false),
    controle_visualiza(0),
    mostra_msg_ini(0),
    linha_ini(11),
    hora_1(0), hora_2(0),
    minuto_1(0), minuto_2(0),
    dia_1(0), dia_2(0),
    mes_1(0), mes_2(0),
    ano_1(0), ano_2(0)
  {}
};

// Hardware e Sensores
struct HardwareState {
  int controle_ldr;                // Controle sensor LDR
  int ldr_max;                     // Valor máximo LDR
  int contador_moedas;             // Contador moedas dispensadas
  int qtd_moedas_dispensar;        // Quantidade dispensar (troco)
  int leitura_rep;                 // Repetições leitura
  long timeout_motor;              // Timeout motor (ms)
  int controle_timeout_motor;      // Controle timeout motor
  bool teste_entrega;              // Teste entrega
  int controle_buzzer;             // Controle buzzer
  short controle_buzzer1;          // Controle auxiliar buzzer
  
  // Construtor com valores padrão
  HardwareState() :
    controle_ldr(0),
    ldr_max(0),
    contador_moedas(0),
    qtd_moedas_dispensar(0),
    leitura_rep(0),
    timeout_motor(0),
    controle_timeout_motor(0),
    teste_entrega(false),
    controle_buzzer(0),
    controle_buzzer1(0)
  {}
};

// Configuração Sistema
struct SystemConfig {
  int first_time;                  // Flag primeira execução
  int tipo_maquina;                // Tipo/modelo máquina
  int status_maquina;              // Status: 1=ativo, 0=inativo
  int em_inicializacao;            // Sistema inicializando
  int altera_estado_notas;         // Estado alteração notas
  int posicao_horario;             // Posição horário
  long pin_parte[6];               // Partes PIN
  unsigned int config_preco_valor[10]; // Config preços
  bool temp_bill_teste;            // Teste bill
  int divide_int;                  // Divisor
  long valor_real;                 // Valor real
  int aux_in;                      // Auxiliar entrada
  int cont_segundos;               // Contador segundos
  
  // Construtor com valores padrão
  SystemConfig() :
    first_time(0),
    tipo_maquina(0),
    status_maquina(1),
    em_inicializacao(0),
    altera_estado_notas(-1),
    posicao_horario(0),
    temp_bill_teste(false),
    divide_int(0),
    valor_real(0),
    aux_in(0),
    cont_segundos(0)
  {
    memset(pin_parte, 0, sizeof(pin_parte));
    memset(config_preco_valor, 0, sizeof(config_preco_valor));
  }
};

#endif
