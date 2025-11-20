/****************************************Copyright (c)*************************************************
**                      Power Vending - A. R. M. Venda Automatica Ltda.
**                             http://www.powervending.com.br
**--------------Informação do Arquivo -----------------------------------------------------------------
** Nome do Arquivo:          SensorQuedaInfra.cpp
** Data Ultima Modificação:  06-09-17
** Ultima Versão:            Sim
** Descrição:                Biblioteca do sensor de queda infravermelho elevador.
**------------------------------------------------------------------------------------------------------
** Criado por:          Marlon Zanardi <dev@powervending.com.br>
** Data de Criação:     05-09-17     
********************************************************************************************************/

#include "SensorQuedaInfra.h"

/*********************************************************************************************************
** Nome da Função:       SensorQuedaInfra
** Descrição:            Função que é o construtor da classe, inicializa a serial de comunicação.
** Parametro:            Não.
** Valor de retorno:     Não.
*********************************************************************************************************/
SensorQuedaInfra::SensorQuedaInfra()
{     
  // Reserva 200 bytes para receber a string.
  string_serial.reserve(1000);
  pinMode(4,OUTPUT);
  digitalWrite(4,HIGH);
  digitalWrite(4,LOW);
  digitalWrite(4,HIGH);
}

/*********************************************************************************************************
** Nome da Função:      task
** Descrição:    	Tarefa que faz a comunicação serial e envia o ping.
** Parametro:           modelo_maquina = Modelo da maquina, temperatura = Temperatura da maquina.
** Valor de retorno:    Não.
*********************************************************************************************************/
void SensorQuedaInfra::task()
{
  // Faz a comunicação serial com o arduino uno.
  com_serial_queda();
  
  // Verifica Timeout de 15 segundos caso não constate verificação do sensor da base.
  tempo_atual_com_ok = millis(); 
  if((tempo_atual_com_ok-time_start_com_ok) > 30000)
  {
    status_com_ok = 0;
    time_start_com_ok = millis();
  }
}

void SensorQuedaInfra::reabre_com()
{
  Serial.println("REABRE COM");
  pinMode(4,OUTPUT);
  digitalWrite(4,HIGH);
  digitalWrite(4,LOW);
  digitalWrite(4,HIGH);
}

/*********************************************************************************************************
** Nome da Função:      get_canal_detectado
** Descrição:    	Método que retorna o canal detectado.
** Parametro:           Não.
** Valor de retorno:    Não.
*********************************************************************************************************/
int SensorQuedaInfra::get_canal_detectado()
{
  // Retorna a variavel.
  return canal_ativo;
}

/*********************************************************************************************************
** Nome da Função:      get_evento_disponivel
** Descrição:    	Método que retorna se existe um evento de leitura detectada disponivel.
** Parametro:           Não.
** Valor de retorno:    Não.
*********************************************************************************************************/
int SensorQuedaInfra::get_evento_disponivel()
{
  // Retorna a variavel.
  return evento_disp;
}

/*********************************************************************************************************
** Nome da Função:      set_evento_disponivel
** Descrição:    	Método que altera o valor da variavel evento_disp.
** Parametro:           Não.
** Valor de retorno:    Não.
*********************************************************************************************************/
void SensorQuedaInfra::set_evento_disponivel(int x)
{
  // Joga o valor na variavel.
  evento_disp = x;
}


/*********************************************************************************************************
** Nome da Função:      teste
** Descrição:           Função que envia o comando para serial.
** Parametro:           Não.
** Valor de retorno:    Não.
*********************************************************************************************************/
void SensorQuedaInfra::teste()
{  
  // Monta a linha do comando da telemetria.
  comando_queda = "TESTE 1";
  // Envia para box o comando.
  queda_envia(comando_queda);  
}

/*********************************************************************************************************
** Nome da Função:      teste
** Descrição:           Função que envia o comando para serial.
** Parametro:           Não.
** Valor de retorno:    Não.
*********************************************************************************************************/
void SensorQuedaInfra::ping_pv()
{  
  // Monta a linha do comando da telemetria.
  comando_queda = "PING PV";
  // Envia para box o comando.
  queda_envia(comando_queda);  
}

/*********************************************************************************************************
** Nome da Função:      liga_motor
** Descrição:           Função que envia o comando para serial.
** Parametro:           Não.
** Valor de retorno:    Não.
*********************************************************************************************************/
void SensorQuedaInfra::leitura_fc(int fc)
{  
  switch(fc)
  {
    case 1:
          // Monta a linha do comando da telemetria.
          comando_queda = "FC_INICIO";
          // Envia para box o comando.
          queda_envia(comando_queda); 
          break;
    case 2:
          // Monta a linha do comando da telemetria.
          comando_queda = "FC_FIM";
          // Envia para box o comando.
          queda_envia(comando_queda); 
          break; 
     
    case 3:
          // Monta a linha do comando da telemetria.
          comando_queda = "FC_ENCODER";
          // Envia para box o comando.
          queda_envia(comando_queda); 
          break;       
  }
}

/*********************************************************************************************************
** Nome da Função:      liga_motor
** Descrição:           Função que envia o comando para serial.
** Parametro:           Não.
** Valor de retorno:    Não.
*********************************************************************************************************/
void SensorQuedaInfra::posiciona_scaner(int pos_scaner)
{  
  switch(pos_scaner)
  {
    case 0:
          // Monta a linha do comando da telemetria.
          comando_queda = "POSICIONA SCANER 0";
          // Envia para box o comando.
          queda_envia(comando_queda); 
          break;
    case 1:
          // Monta a linha do comando da telemetria.
          comando_queda = "POSICIONA SCANER 1";
          // Envia para box o comando.
          queda_envia(comando_queda); 
          break; 
     
    case 2:
          // Monta a linha do comando da telemetria.
          comando_queda = "POSICIONA SCANER 2";
          // Envia para box o comando.
          queda_envia(comando_queda); 
          break;  
    case 3:
          // Monta a linha do comando da telemetria.
          comando_queda = "POSICIONA SCANER 3";
          // Envia para box o comando.
          queda_envia(comando_queda); 
          break;  
    case 4:
          // Monta a linha do comando da telemetria.
          comando_queda = "POSICIONA SCANER 4";
          // Envia para box o comando.
          queda_envia(comando_queda); 
          break;      
  }
}

/*********************************************************************************************************
** Nome da Função:      liga_motor
** Descrição:           Função que envia o comando para serial.
** Parametro:           Não.
** Valor de retorno:    Não.
*********************************************************************************************************/
void SensorQuedaInfra::liga_motor(int motor)
{  
  switch(motor)
  {
    case 1:
          // Monta a linha do comando da telemetria.
          comando_queda = "LIGA_MOTOR_1";
          // Envia para box o comando.
          queda_envia(comando_queda); 
          break;
    case 2:
          // Monta a linha do comando da telemetria.
          comando_queda = "LIGA_MOTOR 2";
          // Envia para box o comando.
          queda_envia(comando_queda); 
          break;      
  }
}

/*********************************************************************************************************
** Nome da Função:      desliga_motor
** Descrição:           Função que envia o comando para serial.
** Parametro:           Não.
** Valor de retorno:    Não.
*********************************************************************************************************/
void SensorQuedaInfra::desliga_motor(int motor)
{  
  switch(motor)
  {
    case 1:
          // Monta a linha do comando da telemetria.
          comando_queda = "DESLIGA_MOTOR 1";
          // Envia para box o comando.
          queda_envia(comando_queda); 
          break;
    case 2:
          // Monta a linha do comando da telemetria.
          comando_queda = "DESLIGA_MOTOR 2";
          // Envia para box o comando.
          queda_envia(comando_queda); 
          break;      
    case 3:
          // Monta a linha do comando da telemetria.
          comando_queda = "MOTOR_1 LIGAR";
          // Envia para box o comando.
          queda_envia(comando_queda); 
          break;      
  }
}

/*********************************************************************************************************
** Nome da Função:      realiza_leitura
** Descrição:           Função que envia o comando para serial.
** Parametro:           Não.
** Valor de retorno:    Não.
*********************************************************************************************************/
void SensorQuedaInfra::realiza_leitura()
{   
  limpa_buffer();
  evento_disp = 0;
  // Monta a linha do comando da telemetria.
  comando_queda = "REALIZA LEITURA";
  // Envia para box o comando.
  queda_envia(comando_queda);  
}

/*********************************************************************************************************
** Nome da Função:      finaliza_leitura
** Descrição:           Função que envia o comando para serial.
** Parametro:           Não.
** Valor de retorno:    Não.
*********************************************************************************************************/
void SensorQuedaInfra::finaliza_leitura()
{   
  limpa_buffer();
  evento_disp = 0;
  // Monta a linha do comando da telemetria.
  comando_queda = "FINALIZA LEITURA";
  // Envia para box o comando.
  queda_envia(comando_queda);  
}

/*********************************************************************************************************
** Nome da Função:     	serialEvent
** Descrição:    	Tafera de interrupção quando a algum evento na serial.
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void SensorQuedaInfra::serialEvent() 
{
  while ( Serial3.available() ) 
  {
    // Pega o novo byte:
    char inChar = (char)Serial3.read();
    // Verifica se é o final da string.
    if (inChar == '\n') {
      string_serial[string_serial.length()-1] = '\0';
      stringComplete = true;
    // Se não incrementa o caractere  
    }else
      string_serial += inChar;    
  }
}

/*********************************************************************************************************
** Nome da Função:     	selecao_tipo
** Descrição:    	Envia o tipo do sensor infra.
** Parametro:           Tipo = 1 - Dois conjuntos(CENCI), 2 - Um conjunto(NewSlim).
** Valor de retorno:    Não
*********************************************************************************************************/
void SensorQuedaInfra::selecao_tipo(int tipo)
{
  // Monta a linha do comando da telemetria.
  comando_queda = "SELECAO_TIPO " + (String)tipo;
  // Envia para box o comando.
  queda_envia(comando_queda);  
}

/*********************************************************************************************************
** Nome da Função:     	limpa_buffer
** Descrição:    	Limpa o buffer Serial.
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void SensorQuedaInfra::limpa_buffer()
{
  // Recebe quantos bytes estao disponiveis na porta.
  unsigned char temp_bf = Serial3.available(); 

  // "Descarta" estes dados.
  for (int i = temp_bf; i>=0; i--)
  {
    Serial3.read();                 
  }
}

/*********************************************************************************************************
** Nome da Função:     	verifica_caractere
** Descrição:    	Função que verifica quantos caracteres existem em uma determinada string.
** Parametro:           qtd_espaco: Quantidade de intervalos(espaços)
                        v_posiao: Parametro do comando.
                        v_crc: Valor do CRC.
** Valor de retorno:    Não
*********************************************************************************************************/
int SensorQuedaInfra::verifica_caractere( char caracter, String comand )
{
  int lmt_string;
  int contador=0;
  
  // Pega o tamanho do comando.
  lmt_string = comand.length();
  
  // Verifica de acordo com o parametro caractere, a quantidade encontrada.
  for(int i=0; i<lmt_string; i++)
  {    
     if(comand[i] == caracter)
     {
        contador++; 
     }
  } 

  // Retorna o contador.  
  return contador;
}

/*********************************************************************************************************
** Nome da Função:     	scaner_posicionamento
** Descrição:    	Funcao que informa se o scaner foi posicionado.
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
int SensorQuedaInfra::scaner_posicionado()
{
  return scaner_chegada;
}

/*********************************************************************************************************
** Nome da Função:     	set_scaner_chegada
** Descrição:    	Funcao que informa se o scaner foi posicionado.
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void SensorQuedaInfra::set_scaner_chegada(int x)
{
  scaner_chegada = x;
}

/*********************************************************************************************************
** Nome da Função:     	com_serial_telemetria
** Descrição:    	Tarefa que faz a leitura da porta serial e faz a comunicação de acordo com os 
                        comandos.
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void SensorQuedaInfra::com_serial_queda()
{
  String resto_comando = " ";
  String palavra_comando = " "; 
  int qtd_espaco;
  char mensagem_string[30];
  String retorna_cmd = " ";
  int first_space;
   
  
  // Verifica a chegada de valores na Serial de comunicação.
  serialEvent();
  
  // Verifica se chegou uma string completa.
  if ( stringComplete ) 
  {
    // Serial de debug.
    //Serial.print("String: ");
    //Serial.println(string_serial);   
    
  
    qtd_espaco = verifica_caractere(' ', string_serial);  
    if ( qtd_espaco > 0 )
    {      
      first_space = string_serial.indexOf(' ');
      if ( first_space > 0 )
      {
        palavra_comando = string_serial.substring(0,first_space);
        resto_comando = string_serial.substring((first_space+1),(string_serial.length()-1));
      }   
      resto_comando.toCharArray(mensagem_string,(resto_comando.length()+1)); 
    }
    
    // Inicio handshack
    if ( string_serial.equals("INICIANDO_SENSORQUEDAINFRA") )
    {      
      queda_envia("SOLICITA VERSAO");
    }

    // Envia versao ativa. 
    if ( palavra_comando.equals("VERSAO_QUEDA") )
    {      
      strcpy(versao_SQI,mensagem_string);
      queda_envia("VERSAO OK");
    }
    
    // Envia versao ativa. 
    if ( palavra_comando.equals("DEBUG") )
    {     
      Serial.print("DEBUG: ");
      Serial.println(mensagem_string);
      
      if ( strcmp(mensagem_string,"POSICAO_OK") == 0 )
      {      
        status_com_ok = 1;   
        time_start_com_ok = millis(); 
        scaner_chegada = 1;        
      }
    }
    
    // Envia versao ativa. 
    if ( string_serial.equals("DETECTOU_CANAL") )
    {      
      // Recebe qual foi o canal detectado.
     // canal_detectado = resto_comando;
      // Indica que existe um evento disponivel para leitura.
      evento_disp = 1;
      
      queda_envia("CANAL OK");
      //Serial.print("CANAL DETECTADO: ");
      //Serial.println(canal_detectado);
    }
    
    // Envia versao ativa. 
    if ( string_serial.equals("DETECTOU_CANAL_0") )
    {
      canal_ativo = 0;
      // Indica que existe um evento disponivel para leitura.
      evento_disp = 1;
    }
    
    // Envia versao ativa. 
    if ( string_serial.equals("DETECTOU_CANAL_1") )
    {
      Serial.println("DETEC CANAL 1");
      canal_ativo = 1;
      // Indica que existe um evento disponivel para leitura.
      evento_disp = 1;
    }
    
    // Envia versao ativa. 
    if ( string_serial.equals("DETECTOU_CANAL_2") )
    {
      //Serial.println("DETEC CANAL 2");
      canal_ativo = 2;
      // Indica que existe um evento disponivel para leitura.
      evento_disp = 1;
    }
    
    // Envia versao ativa. 
    if ( string_serial.equals("DETECTOU_CANAL_3") )
    {
      canal_ativo = 3;
      // Indica que existe um evento disponivel para leitura.
      evento_disp = 1;
    }
    
    // Envia versao ativa. 
    if ( string_serial.equals("DETECTOU_CANAL_4") )
    {
      canal_ativo = 4;
      // Indica que existe um evento disponivel para leitura.
      evento_disp = 1;
    }
    
    // Envia versao ativa. 
    if ( string_serial.equals("DETECTOU_CANAL_5") )
    {
      canal_ativo = 5;
      // Indica que existe um evento disponivel para leitura.
      evento_disp = 1;
    }
    
    // Envia versao ativa. 
    if ( string_serial.equals("DETECTOU_CANAL_6") )
    {
      canal_ativo = 6;
      // Indica que existe um evento disponivel para leitura.
      evento_disp = 1;
    }
    
    // Envia versao ativa. 
    /*if ( string_serial.equals("DETECTOU_CANAL_7") )
    {
      canal_ativo = 7;
      // Indica que existe um evento disponivel para leitura.
      evento_disp = 1;
    }*/
    
    // Envia versao ativa. 
    if ( string_serial.equals("MOTOR_1_LIGADO") )
    {
      Serial.println("MOTOR 1 QUEDA LIGADO");
    }
    
    // Envia versao ativa. 
    if ( string_serial.equals("MOTOR_2_LIGADO") )
    {
      Serial.println("MOTOR 2 QUEDA LIGADO");
    }
    
    // Envia versao ativa. 
    if ( string_serial.equals("MOTOR_1_DESLIGADO") )
    {
      Serial.println("MOTOR 1 QUEDA DESLIGADO");
    }
    
    // Envia versao ativa. 
    if ( string_serial.equals("MOTOR_2_DESLIGADO") )
    {
      Serial.println("MOTOR 2 QUEDA DESLIGADO");
    }
    
    // Finaliza hs.
    if ( string_serial.equals("LEITURA_FINALIZADA") )
    {
      Serial.println("LEITURA F");
      // seta zerro para evento disponivel para leitura.
      evento_disp = 0;
    }
    
    // Envia versao ativa. 
    if ( string_serial.equals("FC_INICIO 1") )
    {
      Serial.println("FC INICIO 1");
      status_com_ok = 1;   
      time_start_com_ok = millis();   
    }
    
    // Envia versao ativa. 
    if ( string_serial.equals("FC_INICIO 0") )
    {
      Serial.println("FC INICIO 0");
      status_com_ok = 1;   
      time_start_com_ok = millis(); 
    }
    
    // Envia versao ativa. 
    if ( string_serial.equals("PING_OK") )
    {
      Serial.println("PING OK");
      status_com_ok = 1;   
      time_start_com_ok = millis(); 
    }
    
    // Envia versao ativa. 
    if ( string_serial.equals("FC_FIM 1") )
    {
      Serial.println("FC FIM 1");
    }
    
    // Envia versao ativa. 
    if ( string_serial.equals("FC_FIM 0") )
    {
      Serial.println("FC FIM 0");
    }
    
    // Envia versao ativa. 
    if ( string_serial.equals("FC_ENCODER 0") )
    {
      Serial.println("FC ENCODER 0");
    }
    
    // Envia versao ativa. 
    if ( string_serial.equals("FC_ENCODER 1") )
    {
      Serial.println("FC ENCODER 1");
    }
    
    // Envia versao ativa. 
    if ( string_serial.equals("POSICIONANDO SCANER 0") )
    {    
      status_com_ok = 1;   
      time_start_com_ok = millis(); 
    }
    
    // Envia versao ativa. 
    if ( string_serial.equals("POSICIONANDO SCANER 1") )
    {
      //Serial.println("POSICIONANDO SCANER 1");
    }
    
    // Envia versao ativa. 
    if ( string_serial.equals("POSICIONANDO SCANER 2") )
    {
      //Serial.println("POSICIONANDO SCANER 2");
    }
    
    // Envia versao ativa. 
    if ( string_serial.equals("POSICIONANDO SCANER 3") )
    {
      //Serial.println("POSICIONANDO SCANER 3");
    }
    
    // Envia versao ativa. 
    if ( string_serial.equals("POSICIONANDO SCANER 4") )
    {
      //Serial.println("POSICIONANDO SCANER 4");
    }
    
    // Envia versao ativa. 
    if ( string_serial.equals("SCANER POSICIONADO") )
    {      
      status_com_ok = 1;   
      time_start_com_ok = millis(); 
      Serial.println("SCANER POSICIONADO");
      scaner_chegada = 1;
    }
    
    // Envia versao ativa. 
    if ( string_serial.equals("FALHA TIMEOUT SCANER") )
    {
      Serial.println("FALHA TIMEOUT SCANER");
    }
    
    // Envia versao ativa. 
    if ( string_serial.equals("FALHA FC SCANER") )
    {
      Serial.println("FALHA FC SCANER");
    }
    
    // Envia versao ativa. 
    if ( string_serial.equals("PEGOU IMA") )
    {
      Serial.println("PEGOU IMA");
    }
    
    // Finaliza hs.
    if ( string_serial.equals("FINALIZA_HS") )
    {            
      Serial.println("HS FINALIZADO");
      //Serial.println("Handshake SQI Finalizado. Versao: " + (String)versao_SQI);
      queda_envia("FINALIZA OK");
      // Altera variavel de controle hs.
      in_ok = 1;
      
      // Limpa o buffer.
      limpa_buffer();
    }
    
    // Limpa a string:
    string_serial = "";
    stringComplete = false;
  }
}

int SensorQuedaInfra::get_status_com_ok()
{
  return status_com_ok;
}

/*********************************************************************************************************
** Nome da Função:     	queda_envia
** Descrição:         	Funcao que escreve na "Serial" para comunicacao com a telemetria.
** Parametro:           comand: Mensagem a ser enviada.
** Valor de retorno:    Não
*********************************************************************************************************/
void SensorQuedaInfra::queda_envia(String comand)
{
  // Escreve na serial.
  Serial3.println(comand);
}
