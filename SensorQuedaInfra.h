/****************************************Copyright (c)*************************************************
**                      Power Vending - A. R. M. Venda Automatica Ltda.
**                             http://www.powervending.com.br
**--------------Informação do Arquivo -----------------------------------------------------------------
** Nome do Arquivo:          SensorQuedaInfra.h
** Data Ultima Modificação:  06-09-17
** Ultima Versão:            Sim
** Descrição:                Biblioteca do sensor de queda infravermelho elevador(Header).          
**------------------------------------------------------------------------------------------------------
** Criado por:          Marlon Zanardi <dev@powervending.com.br>
** Data de Criação:     05-09-17      
********************************************************************************************************/

#ifndef SensorQuedaInfra_h
#define SensorQuedaInfra_h

#include <Arduino.h>

// Constante versão software.
#define V_SFT_Q "v1.01"

// Definição da classe Buzzer
class SensorQuedaInfra
{
  public:
      // Função que é o construtor da classe, pega do buzzer e o delay.
      SensorQuedaInfra();
      
      //Tarefa que faz a comunicação serial e envia o ping.
      void task();
      
      // Tafera de interrupção quando a algum evento na serial.
      void serialEvent();
      
      // Tarefa que faz a leitura da porta serial e faz a comunicação de acordo com os comandos.
      void com_serial_queda();
      
      // Funcao que escreve na "Serial3" para comunicacao com o sensor de queda.
      void queda_envia(String comand);
      
      // Limpa o buffer Serial.
      void limpa_buffer();
            
      // Função que envia o comando teste para serial.
      void teste();      
      
      // Verifica a quantidade de um respectivo caractere.
      int verifica_caractere( char caracter, String comand );
      
      // Envia o tipo do sensor infra.
      void selecao_tipo(int tipo);
      
      // Método que retorna se existe um evento de leitura detectada disponivel.
      int get_evento_disponivel();
      
      // Método que altera o valor da variavel evento_disp.
      void set_evento_disponivel(int x);
      
      // Método que retorna o canal detectado.
      int get_canal_detectado();
      
      void realiza_leitura();
      
      void finaliza_leitura();
      
      void liga_motor(int motor);
      
      void desliga_motor(int motor);
      
      void leitura_fc(int fc);
      
      void posiciona_scaner(int pos_scaner);
      
      int scaner_posicionado();
      
      void set_scaner_chegada(int x);
      
      void reabre_com();
      
      int get_status_com_ok();
      
      void ping_pv();
      
  private:         
      int status_com_ok = 1;  
      long time_start_com_ok = millis(), tempo_atual_com_ok = millis();
      // Comando que ira enviar dados do sensor infravermelho de queda.
      String comando_queda = "";  
      // String que recebe os comandos da serial
      String string_serial = "";  
      // Boleana que informa que a string foi recebida.
      boolean stringComplete = false;    
      // Iniciacao ok.
      int in_ok = 0;    
      // Versao sensor queda infra.
      char versao_SQI[5];
      // Canal detectado
      String canal_detectado;
      // Evento disponivel.
      int evento_disp = 0;
      // canal ativo.
      int canal_ativo = 0;
      // Chegada scaner.
      int scaner_chegada = 0;
};

#endif

