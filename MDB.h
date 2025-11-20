/****************************************Copyright (c)*************************************************
**                      Power Vending - A. R. M. Venda Automatica Ltda.
**                             http://www.powervending.com.br
**--------------Informação do Arquivo -----------------------------------------------------------------
** Nome do Arquivo:          MDB.cpp
** Data Ultima Modificação:  19-10-16
** Ultima Versão:            Sim
** Descrição:                Biblioteca do protocolo(Header).         
**------------------------------------------------------------------------------------------------------
** Criado por:          Marlon Zanardi <dev@powervending.com.br>
** Data de Criação:     12-07-16       
********************************************************************************************************/

#ifndef MDB_h
#define MDB_h

#include <Arduino.h>

// Dados do moedeiro
struct INFO_COIN{                     
  byte feature_level;
  unsigned int code_country;
  unsigned int current_code;
  byte fator_escala;                  // Fator divisão (5)
  byte casas_decimais;                // Casas decimais (2)
  short int tipo_moeda_aceita[2];
  byte tipo_credito[16];              // Valores escala moedas
  byte manufacturer_code[3];
  byte serial_number[12];
  byte model[12];
  byte software_version[2];
  byte optional_features[4];
  byte status_ativo[16];   
};

// Estrutura para armazenamento dos dados do validador de cedulas;
struct INFO_BILL{            
  int feature_level;          // Level do bill.
  int stacker;                // Quantas notas tem no stacker; 
  unsigned int code_country;  // Codigo do pais.
  int fator_escala;           // Fator de divisão(geralmente 5).
  int casas_decimais;         // Casas decimais utilizadas(geralmente 2).
  int stacker_capacidade;     // Capacidade de notas no equipamento.
  int security_level;         // Nivel de seguranca do bill.
  int escrow_ativo;           // Se escrow ativo FF.
  int tipo_credito[16];       // Tipos de notas aceitas.
  int manufacturer_code[3];   // Codigo do fabricante.
  int serial_number[12];      // Numero do serial.
  int model[12];              // Modelo.
  int software_version[2];    // Versao do software
  byte status_ativo[16];      // Status de ativdade dos tipos de cedula.
};

// Estrutura para armazenamento dos dados do cashless;
struct INFO_CASH{             
  int feature_level;          // Level do cashless.
  unsigned int code_country;  // Codigo do pais.
  unsigned int current_code;  // Codigo corrente.
  int fator_escala;           // Fator de divisão(geralmente 5).
  int casas_decimais;         // Casas decimais utilizadas(geralmente 2).
  int max_response;           // Tempo maximo de resposta.
  int manufacturer_code[3];   // Codigo do fabricante.11
  int serial_number[12];      // Numero do serial.
  int model[12];              // Modelo.
  int software_version[2];    // Versao do software
  byte status_ativo[2];       // Status de atividade dos valores max/min.
};

// Estrutura para armazenamento das informações dos tubos.
struct STATUS_TUBO         
{
  byte cheio;              // Se tubo cheio, valor 1.
  byte Status[16];         // Quantidade de moedas nos tubos. [Valor 0= Moeda de menor valor, 2= Segunda moeda de menor valor...].
  byte Status_cofre[10];   // Moedas enviadas para o cofre.[0=Menor valor, 1=Segundo menor valor...].
};

#define ATIVO 1
#define INATIVO 0

#define DEBUG_MDB
//#define DEBUG_BOOT_MDB
//#define DEBUG_MDB_DETAIL
#define DEBUG_ACEITACAO

#define COIN_DEPOSITADO 2
#define BILL_DEPOSITADO 3

// Valores dos correspondentes bits
#define BIT_128  128
#define BIT_64  64
#define BIT_16  16
#define BIT_32  32
#define BIT_8  8
#define BIT_4  4
#define BIT_2  2
#define BIT_1  1

// Cases do tipo de dado vindo do mdb(Aceitação de deposito).
#define CASH_BOX  0
#define TUBES  1
#define NOT_USED  2
#define REJECT  3
#define  BILL_STACKED  4
#define  ESCROW_POSITION  5
#define  BILL_RETURNED  6
#define  BILL_TO_RECYCLER  7
#define  DISABLE_BILL_REJECTED  8
#define  BILL_TO_RECYCLER_MANUAL  9
#define  MANUAL_DISPENSE  10
#define  TRANSF_FROM_RECYCLER  11
#define  ESCROW_STACKED 12
// Coin status
#define ALAVANCA_COIN_COIN 1
//#define NO_CREDIT 3
//#define DEFECTIVE_TUBE 4
//#define DOUBLE_ARRIVAL 5
//#define TUBE_JAM 7
#define CHK_ERROR 8
#define ROUTING_ERROR 9
//#define COIN_RESET 11
#define COIN_JAM 12
#define COIN_REMOVAL 13
// Bill status
//#define BILL_RESET 6
//#define BILL_REMOVED 7
#define BILL_REJECTED 11
#define BILL_RECICLADOR_ESCROW_REQUEST 33
#define BILL_RECICLADOR_FILLED_KEY 47
// Cashless constantes
#define END_SESSION 7
#define VEND_DENIED 6
#define SESSION_CANCEL 4
#define BEGIN_SESSION 3
#define VEND_APPROVED 5
#define JUST_RESET 0


// Definição da classe Buzzer
class MDB
{
  public:
      // Construtor.
      MDB(int pin_rele_mdb);

      // Função que envia por mdb o valor e posicao da venda feita em dinheiro.
      void cash_sale(int pos, int value_sale);
      
      // Funcao MDB utilizada apos credito liberado e compra efetuada.
      void vend_success(int valor_pos);
      
      // Funcao MDB utilizada sempre que e finalizada a acao de compra com cartao.
      void session_complete();
      
      // Função que faz o cancelamento da compra.
      void vend_cancel();
      
      // Funcao MDB que apos um erro de entrega de produto por exemplo, cancela a venda e reporta o valor.
      void vend_failure();
      
      // Habilita a leitura do cashless.
      void reader_enable();
      
      // Funcao que envia os valores maximos e minimos estabelecidos para o cashless
      void cash_max_min();
      
      // Desabilita a leitura do cashless.
      void reader_disable();
      
      // Funcao que escreve na "Serial1" para comunicacao com o MDB.
      void mdb_envia(int msg);
      
      // Funcao que recebe os dados da "Serial1" vindos dos componentes MDB.
      int mdb_recebe();
      
      // Verifica se a dados a serem lidos pela "Serial1" vindo dos componentes MDB.
      bool mdb_avail();
      
      // Função que calcula o CHK para ser enviado.
      int calc_chk(int data[], int n);
      
      // Função que valida o CHK.
      bool validar_chk(int data[], int n);
      
      // Zera valores da verificação de inatividade.
      void zera_poll_dados();

      // Funcao que envia o COIN TYPE, com dados especificos de moedas a serem aceitas.
      void coin_type(int y_1, int y_2);

      // Funcao que verifica, quais moedas devem ficar "Ativas" , faz o calculo e gera os valores para o codigo COIN TYPE.  
      void calc_coin_type();

      // Funcao que atualiza o status do tubo(quantidades de moedas no tubo) quando uma moeda e inserida no tubo.
      void update_TUBEStatus(int tubo);

      // Funcao que atualiza o status do cofre(quantidades de moedas que foram pro cofre) quando uma moeda e inserida no cofre.
      void update_cofre(int tubo);

      /*
        Funcao que verifica o status da moeda inserida.
        0 e 1: TUBES.
        0 e 0: CASH_BOX.
        1 e 0: NOT_USED.
        1 e 1: REJECT.
      */
      int verifica_coin_routing(int mensagem);

      // Funcao que recebe o valor do protocolo MDB 
      void deposito_coin(int mensagem);

      // Funcao que verifica qual tipo de moeda foi depositada(Valores de 0 a 15).  
      int verifica_coin_tybe_deposited(int mensagem);

      // Verifica se é um deposito bill ou coin.
      int verifica_aceitacao(int mensagem);

      // Funcao que recebe o valor de deposito bill do protocolo MDB.
      void deposito_bill(int msg);

      // Tarefa que le o retorno do poll dos equipamentos MDB.
      void mdb_leitura();

      // Tarefa que manda o poll dos equipamentos ativos a cada 99ms.
      void poller_mdb();

      /*
        Funcao que verifica o status da nota inserida.
        0 e 0 e 0: STACKED.
        0 e 0 e 1: ESCROW_POSITION.
        0 e 1 e 0: RETURNED.
        0 e 1 e 1: RECYCLER.
        1 e 0 e 0: DISABLE_BILL_REJECTED.
        1 e 0 e 1: TO RECYCLER.
        1 e 1 e 0: MANUAL DISPENSE.
        1 e 1 e 1: TRANSF_FROM_RECYCLER.
        
      */
     int verifica_bill_routing(int mensagem); 

     // Funcao que verifica qual tipo de nota foi depositada(Valores de 0 a 15). 
     int verifica_bill_tybe_deposited(int mensagem);

     // Funcao que faz a inicializacao do componente BILL.
     void statemachine_bill();

     // Funcao que envia o BILL TYPE, com dados especificos de notas a serem aceitas.
     void bill_type(int y_1, int y_2);

     // Funcao que verifica, quais notas devem ficar "Ativas" , faz o calculo e gera os valores para o codigo BILL TYPE. 
     void calc_bill_type();

     // Tarefa que verifica os componentes MDB ativos, faz o bot necessario e faz a comunicacao permanente do poll.
     void task();
     
     // Tarefa que recebe os 2 bytes de notas ativas.
     void aceitacao(int notas_ativa[], int moedas_ativa[]);
     
     // Funcao que descarta a leitura de um dado da "Serial1".
     void descarta_leitura(int qtd);
     
     // Funcao que faz a inicializacao do componente MDB - COIN.
     void statemachine_coin();
     
     // Função que coloca em um vetor os dados e verifica o chk.
     bool confirma_chk(int mensagem, int qtd_msg);
     
     // Retorna o valor da variavel.
     int get_coin_coin_ativo();
     
     // Altera valor da variavel coin_coin_ativo.
     void set_coin_coin_ativo(int x);
     
     //Função que verifica alguma alteração de status do coin recebido pelo poll.
     void status_coin(int mensagem);
     
     //Função que verifica alguma alteração de status do bill recebido pelo poll.
     void status_bill(int mensagem);
     
     //Funcao que faz a inicializacao do componente CASHLESS.
     void statemachine_cash();
     
     //Função que verifica alguma alteração de status do cash recebido pelo poll.
     void status_cash(int mensagem);
     
     //Função que faz a requisição de cobrança do cashless.
     void vend_request(int valor_produto, int posicao);
     
     //Tarefa que verifica os componentes MDB que estao ligados e os ativa.
     void verifica_equipamentos_mdb_ativos();
     
     // Retorna o valor da variavel.
     int get_valor_depositado();
     
     // Altera valor da variavel valor_depositado.
     void set_valor_depositado(int x);
     
     // Retorna o valor da variavel.
     int get_estado_aceitacao();
     
     // Retorna o valor da variavel.
     int get_inicio_sessao();
     
     // Altera valor da variavel inicio_sessao.
     void set_inicio_sessao(int x);
     
     // Retorna o valor da variavel.
     int get_venda_aprovada();
     
     // Altera valor da variavel venda_aprovada.
     void set_venda_aprovada(int x);
     
     // Retorna o valor da variavel.
     int get_venda_negada();
     
     // Altera valor da variavel venda_negada.
     void set_venda_negada(int x);
     
     // Retorna o valor da variavel.
     int get_bill_disable();
     
     // Altera valor da variavel bill_disable.
     void set_bill_disable(int x);
     
     // Retorna o valor da variavel.
     int get_sessao_finalizada();
     
     // Altera valor da variavel sessao_finalizada.
     void set_sessao_finalizada(int x);
     
     // Método que retorna a estrutura com os dados do equipamento aceitador de moedas.
     INFO_COIN informacoes_coin();
     
     // Método que retorna a estrutura com os dados do equipamento aceitador de notas.
     INFO_BILL informacoes_bill();
     
     // Método que retorna a estrutura com os dados do equipamento cashless.
     INFO_CASH informacoes_cash();
     
     // Método que retorna a estrutura com os dados referentes aos tubos do aceitador de moedas.
     STATUS_TUBO tubos_coin();
     
     // Funcao que liga o rele de alimentação do MDB.
     void rele_liga();
     
     // Funcao que desliga o rele de alimentação do MDB.
     void rele_desliga();
     
     // Altera valor do comando type escrow.
     void set_type_escrow(int type_1, int type_2);
     
     // Faz a dispensa de notas no equipamento reciclador de notas.
     int dispensa_reciclador(int valor_troco);
     
     // Método que desabilita a leitura de moedas.
     void desabilita_coin();
     
     // Método que habilita a leitura de moedas.
     void habilita_coin();
     
     // Método que desabilita a leitura de notas.
     void desabilita_bill();
     
     // Método que habilita a leitura de notas.
     void habilita_bill();
     
     // Tarefa que verifica a entrega do troco pelo comando pay out.
     void pay_out();
     
     // Método que solicita ao moedeiro o troco desejado.
     int entregar_troco(int valor);
     
     // Altera valor da variavel boot_mdb
     void set_boot_mdb(int x);
     
     // Verifica se o equipamento bill é um reciclador.
     bool verifica_reciclador();
     
     // Altera valor da variavel disp_reciclador
     void set_disp_reciclador(int x);
     
     // Envia o codigo MDB para dispensa no reciclador de notas.
     void dispense_recycle(int valor);
     
     // Verifica se o equipamento coin esta ativo.
     bool coin_esta_ativo();
     
     // Verifica se o equipamento bill esta ativo.
     bool bill_esta_ativo();
     
     // Verifica se o equipamento cash esta ativo.
     bool cash_esta_ativo();
     
     // Método que retorna o valor contido nos tubos.
     int valor_tubos();
     
     //Funcao que retira a nota da posicao de escrow e coloca dentro do stacker.
     void recebe_escrow();
     
     //Funcao que retira a nota da posicao de escrow e retorna ela pro usuario.
     void rejeita_escrow();
     
     // Funcao que faz o reset do mdb.
     void reset();
     
     // Função que para a ativade do poller.
     void parar();
     
     // Método que verifica a atividade do poller e caso algum equipamento pare de responder reinicia o mdb.
     void verifica_inatividade();
     
     // Verifica se o equipamento cashless é um nayax.
     bool is_nayax();
     
     // Verifica se o equipamento noteiro é um ITL.
     bool is_itl();
     
     void inicia_notas(bool ativo_cinco, bool ativo_dez, bool ativo_vinte);
     
     bool is_bill_jof();
     
  private:

    // Inicializacao das estruturas com valor 0.
    INFO_COIN info_coin = 
    {0,0,0,0,0,{0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{48,48,48},{48,48,48,48,48,48,48,48,48,48,48,48},{48,48,48,48,48,48,48,48,48,48,48,48},{0,0},{0,0,0,0}};
    INFO_BILL info_bill = 
    {0,0,0,0,0,0,0,0,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0},{0,0}};  
    INFO_CASH info_cash =
    {0,0,0,0,0,0,{0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0},{0,0}};
    STATUS_TUBO Status_tubo = 
    {0,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0}};
     
    //Decodificar os codigos mdb de entrada.
    unsigned short int routing[2];   
    unsigned short int coin_routing[6];      
    unsigned short int bill_routing[6];                
    unsigned short int coin_type_deposited[5];   
    //Recebe o tipo de nota inserida.        
    int bill_type_deposited[5];                     
    
    int data[100];
    int resposta[100];
    //Definicao de valor maximo e minimo.
    unsigned int max_price_1 = 0xFF;          
    unsigned int max_price_2 = 0xFF;
    int type_escrow_1 = 0x00;
    int type_escrow_2 = 0x00;   
    unsigned int min_price_1;
    unsigned int min_price_2;
    int dado_poll[10] = {0,0,0,0,0,0,0,0,0,0};
    int contador_bill;
    int zero_mdb=0;
    int is_jofemar=0;
    int estado_verifica = 0;
    int valor_depositado = 0;
    int inicio_sessao = 0;
    int venda_aprovada = 0;
    int venda_negada = 0;
    int sessao_finalizada = 0;
    int rele_mdb;
    int disp_reciclador = 0;
    int nota_em_custodia = 0;
    int qtd_notas_disp = 0;
    int bill_was_reset = 0;
    int bill_was_disable = 0;
    int bill_disable = 0;
    int just_was_reset = 0;
    int sem_retorno_mdb = 0;
    int estado_inatividade = 0;
    int mdb_sem_equipamentos = 0;
    
    int payout_status = 0;
    int valor_entregue_troco = 0;
    unsigned int tempo_atual_verifica_payout=0 , time_start_verifica_payout = millis();

    unsigned int tempo_atual_poll=0 , time_start_poll=0;
    unsigned int tempo_atual_mdb=0 , time_start_mdb = millis();
    unsigned int tempo_atual_boot_bill=0 , time_start_boot_bill = millis();
    unsigned int tempo_atual_boot_coin=0 , time_start_boot_coin = millis();
    unsigned int tempo_atual_boot_cash=0 , time_start_boot_cash = millis();
    unsigned int tempo_atual_verifica_mdb=0 , time_start_verifica_mdb = millis();
    unsigned int tempo_atual_task_mdb=0 , time_start_task_mdb = millis();
    unsigned int tempo_atual_bill_rst=0 , time_start_bill_rst = millis();
    unsigned int tempo_atual_just_rst=0 , time_start_just_rst = millis();
    unsigned int tempo_atual_disable=0 , time_start_disable = millis();
    unsigned int tempo_atual_verifica_inatividade=0 , time_start_verifica_inatividade = millis();
    unsigned int tempo_atual_billrt=0 , time_start_billrt = millis();

    //Variaveis de controle
    bool itl = 0;
    int controle_bill = 2;
    int controle_coin = 2;
    int controle_cash = 2;
    int estado_aceitacao = 0;
    int mdb_task_ctl = 0;
    int boot_mdb = 0;
    int boot_bill = 0;
    int boot_coin = 0;
    int boot_cash = 0;
    int bill_atv = 0;
    int coin_atv = 0;
    int cash_atv = 0;
    int coin_coin_ativo = 0;
    
    int notas_ativa[2];
    int moedas_ativa[2];
};



#endif

