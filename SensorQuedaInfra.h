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

// Sensor queda infravermel ho
class SensorQuedaInfra
{
  public:
      SensorQuedaInfra();
      
      void task();
      void serialEvent();
      void com_serial_queda();
      void queda_envia(String comand);
      void limpa_buffer();
      void teste();      
      int verifica_caractere( char caracter, String comand );
      void selecao_tipo(int tipo);
      int get_evento_disponivel();
      void set_evento_disponivel(int x);
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
      String comando_queda = "";  
      String string_serial = "";  
      boolean stringComplete = false;    
      int in_ok = 0;    
      char versao_SQI[5];
      String canal_detectado;
      int evento_disp = 0;
      int canal_ativo = 0;
      int scaner_chegada = 0;
};

#endif

