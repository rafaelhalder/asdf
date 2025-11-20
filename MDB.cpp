/****************************************Copyright (c)*************************************************
**                      Power Vending - A. R. M. Venda Automatica Ltda.
**                             http://www.powervending.com.br
**--------------Informação do Arquivo -----------------------------------------------------------------
** Nome do Arquivo:          MDB.cpp
** Data Ultima Modificação:  19-10-16
** Ultima Versão:            Sim
** Descrição:                Biblioteca do protocolo(Código Fonte).         
**------------------------------------------------------------------------------------------------------
** Criado por:          Marlon Zanardi <dev@powervending.com.br>
** Data de Criação:     12-07-16       
********************************************************************************************************/

#include "MDB.h"
#include <avr/wdt.h>

/*********************************************************************************************************
** Nome da Função:      MDB
** Descrição:           Função que é o construtor da classe
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
MDB::MDB(int pin_rele_mdb)
{
  // Inicialização 9 bits de comunicacao do mdb
  Serial1.begin(9600, SERIAL_9N1);
  // Rele MDB
  pinMode(pin_rele_mdb, OUTPUT);
  digitalWrite(pin_rele_mdb, LOW);
  rele_mdb = pin_rele_mdb;
  // Inicialização das variaveis de controle de tempo.
  time_start_boot_coin = millis();
  time_start_boot_bill = millis();
  time_start_mdb = millis();
  // Inicia a verificação de equipamentos ativos no barramento MDB.
  estado_verifica = 1;
}

/*********************************************************************************************************
** Nome da Função:      is_nayax
** Descrição:           Verifica se o equipamento cashless é um nayax.
** Parametro:           Não
** Valor de retorno:    True or False.
*********************************************************************************************************/
bool MDB::is_nayax()
{
  // Verifica o codigo do fabricante.
  for(int i =0; i<3;i++)
  {
     if ( info_cash.manufacturer_code[i] == 'N' || info_cash.manufacturer_code[i] == 'Y' || info_cash.manufacturer_code[i] == 'X')
     {
       return true;
     }
  } 
  return false;
}

void MDB::inicia_notas(bool ativo_cinco, bool ativo_dez, bool ativo_vinte)
{
  for(int i=0; i<10; i++)
  {
    if((info_bill.tipo_credito[i]*info_bill.fator_escala)==500 || (info_bill.tipo_credito[i]*info_bill.fator_escala)==1000 || (info_bill.tipo_credito[i]*info_bill.fator_escala)==2000)
    {
      if((info_bill.tipo_credito[i]*info_bill.fator_escala)==500)
      {
        info_bill.status_ativo[i] = ativo_cinco;
      }
      if((info_bill.tipo_credito[i]*info_bill.fator_escala)==1000)
      {
        info_bill.status_ativo[i] = ativo_dez;
      }
      if((info_bill.tipo_credito[i]*info_bill.fator_escala)==2000)
      {
        info_bill.status_ativo[i] = ativo_vinte;
      }
    }else{
            info_bill.status_ativo[i]= INATIVO;
          }
          
   if(info_bill.escrow_ativo == 0xFF)
   {
     type_escrow_1 = 0x00;
     type_escrow_2 = 0x00;
   }else
   {
      type_escrow_1 = 0x00;
      type_escrow_2 = 0x00;
   } 
        
    calc_bill_type(); 
  } 
}

/*********************************************************************************************************
** Nome da Função:      is_itl
** Descrição:           Verifica se o equipamento noteiro é um ITL.
** Parametro:           Não
** Valor de retorno:    True or False.
*********************************************************************************************************/
bool MDB::is_itl()
{
  // Verifica o codigo do fabricante.
  for(int i =0; i<3;i++)
  {
     if ( info_bill.manufacturer_code[i] == 'T' || info_bill.manufacturer_code[i] == 'L')
     {
       return true;
     }
  } 
  return false;
}

/*********************************************************************************************************
** Nome da Função:      verifica_inatividade
** Descrição:           Método que verifica a atividade do poller e caso algum equipamento pare de responder
                        reinicia o mdb.
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::verifica_inatividade()
{
  switch(estado_inatividade)
  {
    case 0:
      if ( boot_mdb == ATIVO  && mdb_sem_equipamentos == INATIVO )
      {
        if ( sem_retorno_mdb == ATIVO )
        {
          time_start_verifica_inatividade = millis();
          estado_inatividade++;
        }
      }else if( mdb_sem_equipamentos == INATIVO )
      {
        //Serial.println(F("BOOT INATIVO"));
        time_start_verifica_inatividade = millis();
        estado_inatividade = 2;
      }else if( mdb_sem_equipamentos == ATIVO )
      {
        time_start_verifica_inatividade = millis();
        estado_inatividade = 3;
      }
      break;
    case 1:   
      tempo_atual_verifica_inatividade = millis();
      if((tempo_atual_verifica_inatividade-time_start_verifica_inatividade) > 60000)
      {
        Serial.println(F("60 Segundos"));
        if( sem_retorno_mdb == ATIVO )
        {
          Serial.println(F("MDB RESET"));
          delay(5000);
          wdt_enable(WDTO_15MS);  // Reset seguro usando watchdog
          while(1) {}
          sem_retorno_mdb = INATIVO;
          estado_inatividade = 0;
        }
      }
      if( sem_retorno_mdb == INATIVO )
      {
        estado_inatividade = 0;
      }
      break;
    case 2:   
      tempo_atual_verifica_inatividade = millis();
      if((tempo_atual_verifica_inatividade-time_start_verifica_inatividade) > 30000)
      {
        Serial.println(F("30 Segundos"));
        if( boot_mdb == INATIVO )
        {
          delay(5000);
          wdt_enable(WDTO_15MS);  // Reset seguro usando watchdog
          while(1) {}
          Serial.println(F("RESET MDB BOOT"));
          reset();
          estado_inatividade = 0;
        }
      }
      if( boot_mdb == ATIVO || mdb_sem_equipamentos == ATIVO)
      {
        estado_inatividade = 0;
      }
      break;  
    case 3:
      tempo_atual_verifica_inatividade = millis();
      if((tempo_atual_verifica_inatividade-time_start_verifica_inatividade) > 60000)
      {
        //Serial.println("RESET CASE 3");
        reset();
        estado_inatividade = 0;
      }
      break;   
  }
}

/*********************************************************************************************************
** Nome da Função:      parar
** Descrição:           Função que para a ativade do poller.
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::parar()
{
  boot_mdb = 0;
  mdb_task_ctl = 0;
}

/*********************************************************************************************************
** Nome da Função:      reset
** Descrição:           Funcao que faz o reset do mdb.
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::reset()
{  
  // Inicia a verificação de equipamentos ativos no barramento MDB.
  estado_verifica = 1;
  // Retorna para inicio da maquina de estado do boot dos equipamentos.
  controle_bill = 2;
  controle_coin = 2;
  controle_cash = 2;
  // Controle de estados do poller.
  mdb_task_ctl = 0;
  // Zera variaveis de controle de finalizacao de boot.
  boot_mdb = 0;
  boot_bill = 0;
  boot_coin = 0;
  boot_cash = 0;
  // Zera variaveis de controle de equipamentos ativos.
  bill_atv = 0;
  coin_atv = 0;
  cash_atv = 0;
  // Inicialização das variaveis de controle de tempo.
  time_start_boot_coin = millis();
  time_start_boot_bill = millis();
  time_start_mdb = millis();
}

/*********************************************************************************************************
** Nome da Função:      rele_liga
** Descrição:           Funcao que liga o rele de alimentação do MDB.
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::rele_liga()
{  
  digitalWrite(rele_mdb, LOW);
}

/*********************************************************************************************************
** Nome da Função:      rele_desliga
** Descrição:           Funcao que desliga o rele de alimentação do MDB.
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::rele_desliga()
{  
  digitalWrite(rele_mdb, HIGH);
}

/*********************************************************************************************************
** Nome da Função:      statemachine_cash
** Descrição:           Funcao que faz a inicializacao do componente CASHLESS.
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::statemachine_cash()
{  
  #ifdef DEBUG_MDB_DETAIL
  Serial.print(F(" Estado CASH:"));
  Serial.println(controle_cash);
  #endif
    
  tempo_atual_boot_cash = millis();
  if((tempo_atual_boot_cash-time_start_boot_cash) > 50)
  {  
    time_start_boot_cash = millis();
    
    switch(controle_cash)
    {
      //Iniciando o sistema(reset).
      case 0:
            mdb_envia(0x110);
            mdb_envia(0x010);          
            controle_cash++;
            break;
     //Após enviar o reset, verificar o retorno.  
     case 1:
            if (Serial1.read() == 0x100) // Se receber o ACK passa pro proximo passo.
            {
              controle_cash++;
            } else
            {
              controle_cash = 0;  // Caso nao receba o ACK, recomeça o processo.
            }
            break;      
     //Enviar o comando SETUP.         
     case 2:
            data[0] = 0x111;         
            data[1] = 0x000;
            data[2] = 0x003;  
            data[3] = 0x004;         
            data[4] = 0x014;
            data[5] = 0x000;
            data[6] = calc_chk(data,7) ;                    
            for(int cont=0; cont < 7; cont++)
            {
              mdb_envia(data[cont]);      //Enviando os dados.
            }
            controle_cash ++; // Caso nao receba o ACK, recomeça o processo.           
            break;
     //Ler os dados.        
     case 3:  
            //Ler os 23 dados e o CHK.
            if(Serial1.available() < 7) 
            {
              return;
            }
            //Serial1.read();
            //Ler os 23 dados e o CHK.
            for(int i = 0; i < 9; i++)
            {
               if(Serial1.available())
               {
                 data[i] = Serial1.read();
                 #ifdef DEBUG_BOOT_MDB
                  Serial.print(F("Dados SETUP: "));
                  Serial.println(data[i]);
                 #endif             
               }
            }      
            //Valiidar o CHK.
            if(validar_chk(data, 9)) 
            {
              //Checksum ok, manda ACK e proseguir
              mdb_envia(0x000);
              controle_cash++;
             }else {
                      //CHK incorreto, enviar um RET
                      mdb_envia(0x0AA);
                   }            
            info_cash.feature_level         = data[1];
            info_cash.code_country          = data[2];
            info_cash.current_code          = data[3];
            info_cash.fator_escala          = data[4];
            info_cash.casas_decimais        = data[5];
            info_cash.max_response          = data[6];
            break;   
     //SETUP MAX/MIN PRICES  
     case 4:            
            data[0] = 0x111;          
            data[1] = 0x001;
            data[2] = 0x0FF;  
            data[3] = 0x0FF;
            data[4] = 0x000;   
            data[5] = 0x000;
            data[6] = calc_chk(data,7) ;                    
            for(int cont=0; cont < 7; cont++)
            {
              mdb_envia(data[cont]);      //Enviando os dados.
            }
            controle_cash++;
            break;
     //Após enviar o cash max/min, verificar o retorno.  
     case 5:
            if (Serial1.read() == 0x100) // Se receber o ACK passa pro proximo passo.
            {              
              controle_cash++;
            } else
            {
              controle_cash = 4;  // Caso nao receba o ACK, recomeça o processo.
            }
            break;        
     //READER ENABLE        
     case 6:          
           mdb_envia(0x114);
           mdb_envia(0x001);
           mdb_envia(0x015);              
           controle_cash ++;     
           break;
     //Após enviar o cash max/min, verificar o retorno.  
     case 7:
            if (Serial1.read() == 0x100) // Se receber o ACK passa pro proximo passo.
            {              
              controle_cash++;
            } else
            {
              controle_cash = 5;  // Caso nao receba o ACK, recomeça o processo.
            }
            break;      
     //Solicitando informacoes referentes a serial, manufacturer code, etc.         
     case 8:            
            data[0] = 0x117;          
            for(int i=0; i <30;i++)
            {
              data[i+1] = 0x000;
            }            
            data[31] = calc_chk(data,32) ;  
            for(int cont=0; cont < 32; cont++)
            {
              mdb_envia(data[cont]);      //Enviando os dados.
            } 
            controle_cash ++; 
            break;
     case 9:       
            if(Serial1.available() < 12) 
            {
              return;
            }
            for(int i = 0; i < 31; i++)
            {              
               if(Serial1.available())
               {
                 data[i] = Serial1.read(); 
                 #ifdef DEBUG_BOOT_MDB
                   Serial.print(F(" Resposta["));
                   Serial.print(i);
                   Serial.print(F("]:"));
                   Serial.println(data[i]);  
                 #endif   
                 if(i>=0 && i<3)
                 {
                   info_cash.manufacturer_code[i] = data[i]; 
                 }     
                 if(i>=3 && i<15)
                 {                 
                   info_cash.serial_number[i-3] = data[i];
                 } 
                 if(i>=15 && i<27)
                 {
                    info_cash.model[i-15] = data[i];
                 }  
                 if(i>=27 && i<29)
                 {
                   info_cash.software_version[i-27] = data[i];
                 }         
                }               
            }            
            controle_cash++;
            break;           
      case 10:    
           boot_cash = ATIVO;
           #ifdef DEBUG_ACEITACAO
           Serial.println(F("BOOT CASH COMPLETO."));  
           #endif
           if(coin_atv == INATIVO && bill_atv == INATIVO && cash_atv == ATIVO)
           {
             boot_mdb = ATIVO;
           }     
           controle_cash++;
           break;
      //Agora devemos consultar o cashless no minimo a casa 100ms(Poll).          
      case 11:   
           break;           
           
    }
  }
}

/*********************************************************************************************************
** Nome da Função:      statemachine_bill
** Descrição:           Funcao que faz a inicializacao do componente BILL.
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::statemachine_bill()
{      
    #ifdef DEBUG_MDB_DETAIL
    Serial.print(F(" Estado            BILL: "));
    Serial.println(controle_bill);
    #endif
    
    tempo_atual_boot_bill = millis();
    if((tempo_atual_boot_bill-time_start_boot_bill) > 50)
    {
      time_start_boot_bill = millis();
      switch(controle_bill)
      {
        //Iniciando o sistema(reset).
        case 0:
                mdb_envia(0x130);
                mdb_envia(0x030);
                controle_bill++;
                break;
        //Após enviar o reset, verificar o retorno.         
        case 1:
                if(Serial1.read() == 0x100) // Se receber o ACK(0) passa pro proximo passo.
                {
                   mdb_envia(0x000);
                   controle_bill++;
                }else
                {
                   controle_bill=0; // Caso nao receba o ACK, recomeça o processo.
                } 
                break;   
        //Enviando Poll necessario apos o reset.        
       case 2:               
              mdb_envia(0x133);          
              mdb_envia(0x033);                
              controle_bill = 21;     
              break;
        case 21: 
              for(int i = 0; i < 10; i++)
              {
                if(Serial1.available())
                {   
                  data[i] = Serial1.read();
                  mdb_envia(0x000);           //Enviando ACK.
                }
              }
              controle_bill= 3;
              break;     
        //Enviar o comando SETUP.        
        case 3:
                mdb_envia(0x131);
                mdb_envia(0x031);
                controle_bill++; 
                break;
        //Ler os dados.        
        case 4:
               //Ler os 23 dados e o CHK.
              if(Serial1.available() < 15) 
              {
                return;
              }
              contador_bill = 0;
              for(int i = 0; i < 28; i++)
              {
                if(Serial1.available())
                {
                  contador_bill++;
                  data[i] = Serial1.read();
                  #ifdef DEBUG_BOOT_MDB
                  Serial.print(F(" Resposta["));
                  Serial.print(i);
                  Serial.print(F("]:"));
                  Serial.println(data[i]);
                  #endif
                }            
              }      
              //Valiidar o CHK.
              if(validar_chk(data, contador_bill)) 
              {
                //Checksum ok, manda ACK e proseguir
                mdb_envia(0x000);
                controle_bill++;
               }else {
                        //CHK incorreto, enviar um RET
                       mdb_envia(0x0AA);
                    }              
              break; 
       //Armazenando dados na estrutura.       
        case 5:  
            info_bill.feature_level     = data[0];
            info_bill.code_country      = (data[1] << 8 | data[2]);
            if(data[4]>0){
              info_bill.fator_escala      = data[4];
            }
            if(data[5]>0){
              info_bill.casas_decimais      = data[5];
            }
            if(data[7]>0){
              info_bill.stacker_capacidade = data[7];
            }else if(data[8]>0){
              info_bill.stacker_capacidade = data[8];
            }
            if(data[8]>0){
              info_bill.security_level = data[8];
            }else if(data[9]>0){
              info_bill.security_level = data[9];
            }
            info_bill.escrow_ativo = data[10];
            info_bill.security_level = data[9];
            for(int i = 0; i < 12; i++) 
            {
               if(data[i+11]>50)
               {
                 info_bill.tipo_credito[i] = 0; 
               }else{
                       info_bill.tipo_credito[i] = data[i+11]; 
                     }          
            }
            mdb_envia(0x136);
            mdb_envia(0x036);
            delay(50);           
            Serial1.read();
            info_bill.stacker = Serial1.read();
            Serial1.read();
            #ifdef DEBUG_BOOT_MDB
            //Serial.println(info_bill.stacker);
            #endif
            controle_bill++;
            break; 
        //Solicitando informacoes referentes a serial, manufacturer code, etc.       
        case 6:               
             mdb_envia(0x137);
             mdb_envia(0x000);
             mdb_envia(0x037);
             controle_bill++; 
             break;
        //Esperar os 30 Bytes de resposta.            
        case 7:              
            if(Serial1.available() < 30) 
            {
              return;
            }
            for(int i = 0; i < 30; i++)
            {
              if(Serial1.available())
              {
                data[i] = Serial1.read();
                #ifdef DEBUG_BOOT_MDB
                Serial.print(F(" Resposta["));
                Serial.print(i);
                Serial.print(F("]:"));
                Serial.println(data[i]); 
                #else
                delay(10);
                #endif
              }            
            }             
            if(validar_chk(data, 30)) 
            {
              //Checksum ok, manda ACK e proseguir
              mdb_envia(0x000);
              controle_bill++;
             }else {
                      //CHK incorreto, enviar um RET
                     mdb_envia(0x0AA);
                  }                
             break;
        //Armazenar dados na estrutura.         
        case 8:              
            for(int i=0;i<30;i++)
            {
              if(i>=0 && i<3)
              {
               info_bill.manufacturer_code[i] = data[i];
              }                 
              if(i>=3 && i<15)
              {
                info_bill.serial_number[i-3] = data[i];
              }
              if(i>=15 && i<27)
              {
                info_bill.model[i-15] = data[i];
              }   
              if(i>=27 && i<28)
              {
                 info_bill.software_version[i-27] = data[i];
              }   
            }
            // Vai para case verificar se o equipamento é um reciclador.
            controle_bill = 11;
            break;
        //Enviando o bill type, para aceitar as notas e o escrow(sim ou nao).       
        case 9: 
            inicia_notas(0,1,1);
            controle_bill++;
            break;
        //Verificar retorno.        
        case 10:           
            data[0] = Serial1.read();
            if(data[0] == 0x100) 
              {
                //ACK recebido, proseguir.
                mdb_envia(0x000);                
                controle_bill = 18;                  
              }else {
                        //Se não receber o ACK, enviar ao bill o comando novamente.
                        controle_bill--;
                    }
              break;
        case 11:
              itl = 0;
              for(int cont=0; cont<3; cont++)
              {
               #ifdef DEBUG_BOOT_MDB
               Serial.println(info_bill.manufacturer_code[cont]);
               #endif
               if(info_bill.manufacturer_code[cont]==84)  
               {                 
                 itl = 1; 
               }              
              }
              // Se for um reciclador.
              if(info_bill.feature_level >= 2 && itl == 0)
              {
                mdb_envia(0x137);
                mdb_envia(0x003);
                mdb_envia(0x03A);
                controle_bill++;
              // Caso nao seja um reciclador.  
              }else
              {
                controle_bill = 9;
              } 
              break;
        //Esperar os 30 Bytes de resposta.            
        case 12:                
              if(Serial1.available() < 3) 
              {
                return;
              }
              for(int i = 0; i < 3; i++)
              {
                if(Serial1.available())
                {
                  data[i] = Serial1.read(); 
                  #ifdef DEBUG_BOOT_MDB
                  Serial.print(F(" Resposta["));
                  Serial.print(i);
                  Serial.print(F("]:"));
                  Serial.println(data[i]); 
                  #else
                  delay(10); 
                  #endif 
                }            
              }             
              if(validar_chk(data, 3)) 
              {
                //Checksum ok, manda ACK e proseguir
                mdb_envia(0x000);
                controle_bill++;
               }else {
                        //CHK incorreto, enviar um RET
                       mdb_envia(0x0AA);
                    }                
               break;
        case 13:
              data[0] = 0x137;          
              data[1] = 0x004;
              data[2] = 0x000;  
              data[3] = 0x000;
              data[4] = 0x001;  
              data[5] = 0x003;
              data[6] = 0x000;  
              data[7] = 0x000;
              data[8] = 0x000;  
              data[9] = 0x000;
              data[10] = 0x000;  
              data[11] = 0x000;
              data[12] = 0x000; 
              data[13] = 0x000;
              data[14] = 0x000;  
              data[15] = 0x000;
              data[16] = 0x000;  
              data[17] = 0x000;
              data[18] = 0x000;
              data[19] = 0x000;
              data[20] = calc_chk(data,21) ;                           
              for(int cont=0; cont < 21; cont++)
              {
                mdb_envia(data[cont]);      //Enviando os dados.
              }  
              delay(30); 
              #ifdef DEBUG_BOOT_MDB              
              Serial.print(F("MS"));
              Serial.println(Serial1.read());
              #else
              Serial1.read();
              #endif
              mdb_envia(0x000);
              controle_bill++;  
              break;
        case 14:  
               mdb_envia(0x137);
               mdb_envia(0x005);
               mdb_envia(0x03C);
               controle_bill++;      
               break;
        case 15:    
              for(int i = 0; i < 35; i++)
              {
                if(Serial1.available())
                {
                  data[i] = Serial1.read(); 
                  #ifdef DEBUG_BOOT_MDB
                  Serial.print(F(" Resposta["));
                  Serial.print(i);
                  Serial.print(F("]:"));
                  Serial.println(data[i]);
                  #endif
                }            
              }             
              if(validar_chk(data, 35)) 
              {
                //Checksum ok, manda ACK e proseguir
                mdb_envia(0x000);
                controle_bill = 9;
               }else {
                        //CHK incorreto, enviar um RET
                       mdb_envia(0x0AA);
                    }                
               break;
         case 20:  
               mdb_envia(0x137);
               mdb_envia(0x001);
               mdb_envia(0x002);
               mdb_envia(0x000);
               mdb_envia(0x000);
               mdb_envia(0x000);
               mdb_envia(0x03A);
               controle_bill = 16;    
               break;        
         case 16:  
               mdb_envia(0x137);
               mdb_envia(0x002);
               mdb_envia(0x039);
               controle_bill++;      
               break;
        case 17:    
              if(Serial1.available() < 34) 
              {
                return;
              }
              for(int i = 0; i < 34; i++)
              {
                if(Serial1.available())
                {
                  data[i] = Serial1.read(); 
                  #ifdef DEBUG_BOOT_MDB
                  Serial.print(F(" Resposta["));
                  Serial.print(i);
                  Serial.print(F("]:"));
                  Serial.println(data[i]);        
                  #endif   
                }            
              }             
              if(validar_chk(data, 34)) 
              {
                //Checksum ok, manda ACK e proseguir
                mdb_envia(0x000);
                controle_bill = 9;
               }else {
                        //CHK incorreto, enviar um RET
                       mdb_envia(0x0AA);
                    }                
               break;        
        //Processo de boot bill completo!
        case  18:                  
               controle_bill++; 
               boot_mdb = ATIVO; 
               boot_bill = ATIVO;
               #ifdef DEBUG_ACEITACAO
               Serial.println(F("BOOT BILL COMPLETO."));  
               #endif
               break;
        //Agora devemos consultar o bill no minimo a casa 100ms(Poll).          
        case 19:                    
              break;   
      
    }
   }    
}

/*********************************************************************************************************
** Nome da Função:      statemachine_coin
** Descrição:           Funcao que faz a inicializacao do componente MDB - COIN.
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::statemachine_coin()
{ 
  int msg;
  int troco = 0x000;

  #ifdef DEBUG_MDB_DETAIL
  Serial.print(F(" Estado COIN: "));
  Serial.println(controle_coin);
  #endif
  
  tempo_atual_boot_coin = millis();
  if((tempo_atual_boot_coin-time_start_boot_coin) > 50)
  {  
    time_start_boot_coin = millis();
    switch (controle_coin)
    {
      //Iniciando o sistema(reset).
      case 0:      
        mdb_envia(0x108);
        mdb_envia(0x008);
        controle_coin++;
        break;
      //Após enviar o reset, verificar o retorno.  
      case 1:
        if (Serial1.read() == 0x100) // Se receber o ACK passa pro proximo passo.
        {
          mdb_envia(0x000);
          controle_coin++;
        } else
        {
          controle_coin = 0;  // Caso nao receba o ACK, recomeça o processo.
        }
        break;
       //Consultar o moedeiro(Poll). 
      case 2:
        mdb_envia(0x10B);
        mdb_envia(0x00B); 
        controle_coin++;
        break;
      //Esperar até o JUST RESET seja enviado.  
      case 3:         
        //Verificar se o primeiro dado recebido é um ACK.
        //Ler e discartar caso seja.
        if(Serial1.peek() == 0x100)
        {
           Serial1.read();  
           return;
         }
         //Ler os dois bytes e verificar se é JUST RESET.
         for(int i = 0; i < 2; i++) 
         {
           data[i] = Serial1.read();
         }         
         
        mdb_envia(0x000);
        controle_coin++;         
        break;  
      //Enviar o comando SETUP.         
      case 4:      
        mdb_envia(0x109);
        mdb_envia(0x009);     
        controle_coin++;
        break;
      //Esperar as respostas. 
      case 5:
        //Esperar os 24 Bytes de resposta.    
        if(Serial1.available() < 15) 
        {
          return;
        }  
        //Ler os 23 dados e o CHK.
        for(int i = 0; i < 24; i++)
        {          
          resposta[i] = Serial1.read();
          delay(15);
          #ifdef DEBUG_BOOT_MDB
          Serial.print(F("Dados SETUP: "));        
          Serial.println(resposta[i]);
          #endif
        }    
        zero_mdb = 0;
        for(int i = 7; i < 23; i++) 
        {           
           if(resposta[i]>100)
           {
             info_coin.tipo_credito[zero_mdb] = 0; 
           }else{
                   info_coin.tipo_credito[zero_mdb] = resposta[i]; 
                 }
           zero_mdb++;  
        }
        mdb_envia(0x000);
        controle_coin++;             
        break;
      //Salvar informações do setup.
      case 6:   
          info_coin.feature_level         = resposta[0];
          info_coin.code_country          = resposta[1];
          info_coin.current_code          = resposta[2];
          info_coin.fator_escala          = resposta[3];
          info_coin.casas_decimais        = resposta[4];
          info_coin.tipo_moeda_aceita[0]  = resposta[5];
          info_coin.tipo_moeda_aceita[1]  = resposta[6];
          controle_coin++;
          break;
      //Enviando comando TUBE STATUS.   
      case 7:      
        mdb_envia(0x10A);
        mdb_envia(0x00A);
        controle_coin++;
        break;
      //Esperar respostas.  
      case 8:    
         if(Serial1.available() < 18) 
         {
           return;
         }  
         //Ler os 18 dados e o CHK.
         for(int i = 0; i < 19; i++)
         {           
           data[i] = Serial1.read();
           delay(15);
           #ifdef DEBUG_BOOT_MDB
           Serial.print(F("Dados TUBE STATUS: "));
           Serial.println(data[i]);
           #endif
         }
         Status_tubo.cheio = (data[0] << 8 | data[1]);
         for(int i=0; i < 19; i++) 
         {
           Status_tubo.Status[i] = data[i+2];
         }    
         controle_coin++;      
         break;      
      //Salvar dados do identification.
      case 9:
         data[0] = 0x10F;
         data[1] = 0x000;
         data[2] = calc_chk(data,3) ;
         for(int cont=0; cont < 3; cont++)
         {
           mdb_envia(data[cont]);   
         } 
         delay(100);       
         for(int i = 0; i < 33; i++)
        {        
           if(Serial1.available())
           {
              data[i] = Serial1.read(); 
              #ifdef DEBUG_BOOT_MDB
              Serial.print(F(" Resposta["));
              Serial.print(i);
              Serial.print(F("]:"));
              Serial.println(data[i]);
              #else
              delay(10);
              #endif
           }                
        }
        for(int i=0;i<33;i++)
          {
            if(i>=0 && i<3)
            {
               info_coin.manufacturer_code[i] = data[i];
               if(info_coin.manufacturer_code[i]=='J')
               {
                   is_jofemar = 1;
               }
            }
          if(i>=3 && i<15)
             {
               info_coin.serial_number[i-3] = data[i];
             }
             if(i>=15 && i<27)
            {
              info_coin.model[i-15] = data[i];
            }   
            if(i>=27 && i<29)
            {
               info_coin.software_version[i-27] = data[i];
            }   
            if(i>=29 && i<33)
            {
              info_coin.optional_features[i-29] = data[i];
            }
          }    
         //Mandar ACK e proseguir
         mdb_envia(0x000);
         controle_coin++;
         break;  
      //Enviando comando COIN TYPE.   
      case  10:      
        calc_coin_type();        
        controle_coin = 12;
        break;
      //Esperar o ACK do moedeiro.  
      case  11:
        data[0] = Serial1.read();
        if(data[0] == 0x100) 
          {
            //ACK recebido, proseguir.
            controle_coin++;
          }else {
                    //Se não receber o ACK, enviar ao moedeiro o comando novamente.
                    data[1] = Serial1.read(); 
                    #ifdef DEBUG_BOOT_MDB
                    Serial.print(F("Erro: Aguardando ACK, dado recebido: "));
                    Serial.println(data[0], HEX);
                    #endif
                    controle_coin--;
                }
          break; 
      //Agora ja é possivel fazer a dispense de uma moeda.   
      case  12:        
        descarta_leitura(2);
        mdb_envia(0x000); 
        controle_coin = 16;
        boot_coin = ATIVO;
        #ifdef DEBUG_ACEITACAO
        Serial.println(F("BOOT COIN COMPLETO."));  
        #endif
        if(coin_atv == ATIVO && bill_atv == INATIVO)
        {
          boot_mdb = ATIVO;
        }
        break;   
      //Agora devemos consultar o moedeiro no minimo a cada 100ms(Poll).  
      case  16:  
            break;
    }    
  } 
}  

/*********************************************************************************************************
** Nome da Função:     	mdb_cash_sale
** Descrição:          	Função que envia por mdb o valor e posicao da venda feita em dinheiro.
** Parametro:           pos: Posição onde foi realizada a compra.
                        value_sale: Valor da venda do produto.
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::cash_sale(int pos, int value_sale)
{
  data[0] = 0x113;        
  data[1] = 0x005;
  data[2] = (value_sale/256);
  data[3] = (value_sale%256);
  data[4] = (pos/256);
  data[5] = (pos%256);
  data[6] = calc_chk(data,7) ;                    
  for(int cont=0; cont < 7; cont++)
  {
    mdb_envia(data[cont]);      //Enviando os dados.
  } 
}

/*********************************************************************************************************
** Nome da Função:     	mdb_vend_succes
** Descrição:    	Funcao MDB utilizada apos credito liberado e compra efetuada.
** Parametro:           valor_pos: Valor da venda do produto.
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::vend_success(int valor_pos)
{
  //Vend Success
  data[0] = 0x113;          
  data[1] = 0x002;
  data[2] = (valor_pos/256);  
  data[3] = (valor_pos%256); 
  data[4] = calc_chk(data,5) ;                    
  for(int cont=0; cont < 5; cont++)
  {
    mdb_envia(data[cont]);      //Enviando os dados.
  } 
}  

/*********************************************************************************************************
** Nome da Função:     	mdb_session_complete
** Descrição:         	Funcao MDB utilizada sempre que e finalizada a acao de compra com cartao.
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::session_complete()
{
  data[0] = 0x113;          //Session Complete
  data[1] = 0x004;
  data[2] = calc_chk(data,3) ;                    
  for(int cont=0; cont < 3; cont++)
  {
    mdb_envia(data[cont]);      //Enviando os dados.
  }
}

/*********************************************************************************************************
** Nome da Função:     	mdb_cancel_vend
** Descrição:         	Função que faz o cancelamento da compra.
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::vend_cancel()
{
   mdb_envia(0x113);
   mdb_envia(0x001);
   mdb_envia(0x014);    
}

/*********************************************************************************************************
** Nome da Função:     	mdb_vend_failure
** Descrição:          	Funcao MDB que apos um erro de entrega de produto por exemplo, cancela a venda e 
                        estorna o valor.
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::vend_failure()
{
  data[0] = 0x113;        
  data[1] = 0x003;
  data[2] = calc_chk(data,3) ;                    
  for(int cont=0; cont < 3; cont++)
  {
    mdb_envia(data[cont]);      //Enviando os dados.
  }  
}

/*********************************************************************************************************
** Nome da Função:     	mdb_reader_enable
** Descrição:         	Habilita a leitura do cashless.
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::reader_enable()
{
   mdb_envia(0x114);
   mdb_envia(0x001);
   mdb_envia(0x015);
   delay(15);
   Serial1.read();
   mdb_envia(0x000);
}

/*********************************************************************************************************
** Nome da Função:     	cash_max_min
** Descrição:          	Funcao que envia os valores maximos e minimos estabelecidos para o cashless
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::cash_max_min()
{
    data[0] = 0x111;          
    data[1] = 0x001;
    data[2] = max_price_1;  
    data[3] = max_price_2;
    data[4] = min_price_1;  
    data[5] = min_price_2;
    data[6] = calc_chk(data,7) ;                    
    for(int cont=0; cont < 7; cont++)
    {
      mdb_envia(data[cont]);      //Enviando os dados.
    }
    delay(20);
    Serial1.read();
    mdb_envia(0x000);
}

/*********************************************************************************************************
** Nome da Função:     	mdb_reader_disable
** Descrição:          	Desabilita a leitura do cashless.
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::reader_disable()
{
   mdb_envia(0x114);
   mdb_envia(0x000);
   mdb_envia(0x014);
   delay(10);
   Serial1.read();
   mdb_envia(0x000);
}

/*********************************************************************************************************
** Nome da Função:      vend_request
** Descrição:           Função que faz a requisição de cobrança do cashless.
** Parametro:           valor_produto: Valor da cobrança.
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::vend_request(int valor_produto, int posicao)
{
  //Vend request para cashless.
  data[0] = 0x113;          
  data[1] = 0x000;
  data[2] = (valor_produto/256);           
  data[3] = (valor_produto%256);
  data[4] = (posicao/256); 
  data[5] = (posicao%256);  
  data[6] = calc_chk(data,7) ;                    
  for(int cont=0; cont < 7; cont++)
  {
    mdb_envia(data[cont]);      //Enviando os dados.
  }
}

/*********************************************************************************************************
** Nome da Função:     	mdb_envia
** Descrição:         	Funcao que escreve na "Serial1" para comunicacao com o MDB.
** Parametro:           msg: Mensagem a ser enviada.
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::mdb_envia(int msg)
{
  Serial1.write(msg);
}

/*********************************************************************************************************
** Nome da Função:     	mdb_recebe
** Descrição:          	Funcao que recebe os dados da "Serial1" vindos dos componentes MDB.
** Parametro:           Não
** Valor de retorno:    retorna o valor recebido pela serial ou -1 caso não tenha recebido nenhum byte.
*********************************************************************************************************/
int MDB::mdb_recebe()
{
  int msg;
  if (mdb_avail())
  {
    msg = Serial1.read();
    return msg;
  }
  return -1;
}

/*********************************************************************************************************
** Nome da Função:     	mdb_avail
** Descrição:         	Verifica se a dados a serem lidos pela "Serial1" vindo dos componentes MDB.
** Parametro:           Não
** Valor de retorno:    retorna se a algum byte para ser lido na "Serial1"
*********************************************************************************************************/
bool MDB::mdb_avail()
{
  return Serial1.available();
}

/*********************************************************************************************************
** Nome da Função:     	calc_chk
** Descrição:         	Função que calcula o CHK para ser enviado.
** Parametro:           data[]: Vetor com os dados que serão calculados
                        n: numero de elementos do vetor.
** Valor de retorno:    Valor em inteiro do calculo do CRC, cortando para 1 byte.
*********************************************************************************************************/
int MDB::calc_chk(int data[], int n) 
{
  int chk = 0x000;

  for (int i = 0; i < n - 1; i++)
  {
    chk += data[i];
  } 
  chk &= 0x0FF;
  return chk;
}

/*********************************************************************************************************
** Nome da Função:     	validar_chk
** Descrição:         	Função que valida o CHK.
** Parametro:           data[]: Vetor com os dados que serão validados
                        n: numero de elementos do vetor.
** Valor de retorno:    1: CHK correto, 0: CHK incorreto
*********************************************************************************************************/
bool MDB::validar_chk(int data[], int n)   
{
    unsigned int chk = calc_chk(data, n); 
    data[n-1] &= 0x0FF;
    return (chk == data[n-1]);    
}

/*********************************************************************************************************
** Nome da Função:       zera_poll_dados
** Descrição:           Zera valores da verificação de inatividade.
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::zera_poll_dados()
{
  for(int i=0; i<10; i++)
  {
    dado_poll[i] = 0;
  }
}

/*********************************************************************************************************
** Nome da Função:      desabilita_coin
** Descrição:           Método que desabilita a leitura de moedas.
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::desabilita_coin()
{
  coin_type(0x00,0x00);
}

/*********************************************************************************************************
** Nome da Função:      habilita_coin
** Descrição:           Método que habilita a leitura de moedas.
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::habilita_coin()
{
  calc_coin_type(); 
}

/*********************************************************************************************************
** Nome da Função:      desabilita_coin
** Descrição:           Método que desabilita a leitura de moedas.
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::desabilita_bill()
{
  bill_type(0x00,0x00);
}

/*********************************************************************************************************
** Nome da Função:      habilita_coin
** Descrição:           Método que habilita a leitura de notas.
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::habilita_bill()
{
  calc_bill_type(); 
}

/*********************************************************************************************************
** Nome da Função:      coin_type
** Descrição:           Funcao que envia o COIN TYPE, com dados especificos de moedas a serem aceitas.
** Parametro:           y_1 primeiro byte de moedas aceitas, y_2 segundo byte de moedas aceitas.
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::coin_type(int y_1, int y_2)
{
  int data[20];
  data[0] = 0x10C;          
  data[1] = y_1;
  data[2] = y_2;  
  data[3] = 0x00;         
  data[4] = 0x00;
  //Calculando CHK necessario.
  data[5] = calc_chk(data, 6);
  for (int cont = 0; cont < 6; cont++)
  {
    mdb_envia(data[cont]);      //Enviando os dados.
  }
  Serial1.read();
}

/*********************************************************************************************************
** Nome da Função:      calc_coin_type
** Descrição:           Funcao que verifica, quais moedas devem ficar "Ativas" , faz o calculo e gera os 
*                       valores para o codigo COIN TYPE.  
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::calc_coin_type()
{
  int y_1 = 0;
  int y_2 = 0;
  int mlt = 1;
  
  for(int i=0; i<16; i++)
  {
    if(i<8)
    {
      if(info_coin.status_ativo[i]==ATIVO)
      {
        y_1 = y_1 + mlt;
      }
    }else if(info_coin.status_ativo[i]==ATIVO){
            y_2 = y_2 + mlt;
          }     
    mlt = mlt*2;    
    if(i==7)
    {
      mlt = 1;
    }   
  }
  //Chama a funcao que envia o codigo MDB.
  coin_type(y_2,y_1);
}

/*********************************************************************************************************
** Nome da Função:      update_TUBEStatus
** Descrição:           Funcao que atualiza o status do tubo(quantidades de moedas no tubo) quando uma 
*                       moeda e inserida no tubo. 
** Parametro:           tubo, tubo no qual foi inserido uma moeda.
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::update_TUBEStatus(int tubo)
{
   Status_tubo.Status[tubo] = Status_tubo.Status[tubo]+1;
}

/*********************************************************************************************************
** Nome da Função:      update_cofre
** Descrição:           Funcao que atualiza o status do cofre(quantidades de moedas que foram pro cofre) 
*                       quando uma moeda e inserida no cofre.
** Parametro:           Tubo, tubo no qual é o tipo da moeda que foi pro cofre.
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::update_cofre(int tubo)
{
  Status_tubo.Status_cofre[tubo] = Status_tubo.Status_cofre[tubo]+1;
} 

/*********************************************************************************************************
** Nome da Função:      verifica_coin_routing
** Descrição:           Funcao que verifica o status da moeda inserida.
                        0 e 1: TUBES.
                        0 e 0: CASH_BOX.
                        1 e 0: NOT_USED.
                        1 e 1: REJECT.
** Parametro:           mensagem: valor vindo do mdb.
** Valor de retorno:    Não
*********************************************************************************************************/
int MDB::verifica_coin_routing(int mensagem)
{
  int coin_deposited;
  
  coin_routing[0] = mensagem & BIT_32;
  coin_routing[1] = mensagem & BIT_16;
  //Moeda foi para o tubo.
  if(coin_routing[0] == 0 && coin_routing[1] == BIT_16)
  {
    coin_deposited = TUBES;
    return coin_deposited;
  }
  //Moeda foi para o cofre.
  if(coin_routing[0] == 0 && coin_routing[1] == 0)
  {
    coin_deposited = CASH_BOX;
    return coin_deposited;
  }
  //Moeda nao utilizada.
  if(coin_routing[0] == 1 && coin_routing[1] == 0)
  {
    coin_deposited = NOT_USED;
    return coin_deposited;
  }
  //Moeda Rejeitada/Desconhecida.
  if(coin_routing[0] == 1 && coin_routing[1] == 1)
  {
    coin_deposited = REJECT;
    return coin_deposited;
  }
}

/*********************************************************************************************************
** Nome da Função:      verifica_coin_tybe_deposited
** Descrição:           Funcao que verifica qual tipo de moeda foi depositada(Valores de 0 a 15).  
** Parametro:           mensagem: valor vindo do mdb.
** Valor de retorno:    Não
*********************************************************************************************************/
int MDB::verifica_coin_tybe_deposited(int mensagem)
{
  int valor;
   //Verfica os bits da mensagem.
   coin_type_deposited[0] = mensagem&8;
   coin_type_deposited[1] = mensagem&4;
   coin_type_deposited[2] = mensagem&2;
   coin_type_deposited[3] = mensagem&1;
   valor = 0;    
   for(int i=0; i<4;i++)
   {
     valor = valor + coin_type_deposited[i];    //Recebe o valor do tubo da moeda.
   }
   return valor;  //Retorna o valor.
}  


/*********************************************************************************************************
** Nome da Função:      verifica_aceitacao
** Descrição:           Verifica se é um deposito bill ou coin
** Parametro:           mensagem: valor vindo do mdb.
** Valor de retorno:    Não
*********************************************************************************************************/
int MDB::verifica_aceitacao(int mensagem)
{
  int destino_mdb;
  
  routing[0] = mensagem & BIT_128;
  routing[1] = mensagem & BIT_64;
  routing[2] = mensagem & BIT_32;
  routing[3] = mensagem & BIT_16;
  
  //Deposito coin.
  if(routing[0] == 0 && routing[1] == BIT_64)
  {
    destino_mdb = COIN_DEPOSITADO;
    return destino_mdb;
  }
  //Deposito nota.
  if(routing[0] == BIT_128)
  {
    destino_mdb = BILL_DEPOSITADO;
    return destino_mdb;
  }  

  return 0;
}  

/*********************************************************************************************************
** Nome da Função:      verifica_bill_routing
** Descrição:           Funcao que verifica o status da nota inserida.
                          0 e 0 e 0: STACKED.
                          0 e 0 e 1: ESCROW_POSITION.
                          0 e 1 e 0: RETURNED.
                          0 e 1 e 1: RECYCLER.
                          1 e 0 e 0: DISABLE_BILL_REJECTED.
                          1 e 0 e 1: TO RECYCLER.
                          1 e 1 e 0: MANUAL DISPENSE.
                          1 e 1 e 1: TRANSF_FROM_RECYCLER.
** Parametro:           mensagem: valor vindo do mdb.
** Valor de retorno:    Não
*********************************************************************************************************/
int MDB::verifica_bill_routing(int mensagem)
{
  int bill_deposited;
  
  bill_routing[0] = mensagem & BIT_64;
  bill_routing[1] = mensagem & BIT_32;
  bill_routing[2] = mensagem & BIT_16;  
  
  //Nota foi para o armazenamento.
  if(bill_routing[0] == 0 && bill_routing[1] == 0 && bill_routing[2] == 0)
  {
    bill_deposited = BILL_STACKED;
    return bill_deposited;
  }
  //Nota em posicao de ESCROW.(Retida ate finalizacao da compra).
  if(bill_routing[0] == 0 && bill_routing[1] == 0 && bill_routing[2] == BIT_16)
  {
    bill_deposited = ESCROW_POSITION;    
    return bill_deposited;
  }
  //Nota inserida errada ou desconhecida.
  if(bill_routing[0] == 0 && bill_routing[1] == BIT_32 && bill_routing[2] == 0)
  {
    bill_deposited = BILL_RETURNED;
    return bill_deposited;
  }
  //Reciclador(Nivel 3+).
  if(bill_routing[0] == 0 && bill_routing[1] == BIT_32 && bill_routing[2] == BIT_16)
  {
    bill_deposited = BILL_TO_RECYCLER;
    return bill_deposited;
  }
  //Nota rejeitada.
  if(bill_routing[0] == BIT_64 && bill_routing[1] == 0 && bill_routing[2] == 0)
  {
    bill_deposited = DISABLE_BILL_REJECTED;
    return bill_deposited;
  }
  //Reciclador(MANUAL-FILL)
  if(bill_routing[0] == BIT_64 && bill_routing[1] == 0 && bill_routing[2] == BIT_16)
  {
    bill_deposited = BILL_TO_RECYCLER_MANUAL;
    return bill_deposited;
  }
  //Dispensa manual
  if(bill_routing[0] == BIT_64 && bill_routing[1] == BIT_32 && bill_routing[2] == 0)
  {
    bill_deposited = MANUAL_DISPENSE;
    return bill_deposited;
  }
  if(bill_routing[0] == BIT_64 && bill_routing[1] == BIT_32 && bill_routing[2] == BIT_16)
  {
    bill_deposited = TRANSF_FROM_RECYCLER;
    return bill_deposited;
  }
} 

/*********************************************************************************************************
** Nome da Função:      verifica_bill_tybe_deposited
** Descrição:           Funcao que verifica qual tipo de nota foi depositada(Valores de 0 a 15). 
** Parametro:           mensagem: valor vindo do mdb.
** Valor de retorno:    Não
*********************************************************************************************************/
int MDB::verifica_bill_tybe_deposited(int mensagem)
{
  int valor;
  //Verfica os bits da mensagem.
   bill_type_deposited[0] = mensagem&BIT_8;
   bill_type_deposited[1] = mensagem&BIT_4;
   bill_type_deposited[2] = mensagem&BIT_2;
   bill_type_deposited[3] = mensagem&BIT_1;  
  
   valor = 0;    
   for(int i=0; i<4;i++)
   {
     valor = valor + bill_type_deposited[i];    //Recebe o valor do tubo da moeda.
   }
   
   return valor;  //Retorna o valor.
} 

/*********************************************************************************************************
** Nome da Função:      descarta_leitura
** Descrição:           Funcao que descarta a leitura de um dado da "Serial1".
** Parametro:           qtd: quantidade de leituras a descartar.
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::descarta_leitura(int qtd)
{
  for(int i=0;i<qtd;i++)
  {
    mdb_recebe();
    delay(15);
  }
}

/*********************************************************************************************************
** Nome da Função:      verifica_coin_tybe_deposited
** Descrição:           Funcao que recebe o valor do protocolo MDB 
** Parametro:           mensagem: valor vindo do mdb.
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::deposito_coin(int mensagem)
{
  int coin_deposited;
  int valor;

  //Chama a funcao que verifica para onde foi a moeda.
  coin_deposited = verifica_coin_routing(mensagem);
  
  switch(coin_deposited)
  {
    //Moeda depositada no cofre.
    case CASH_BOX:    
          if(confirma_chk(mensagem, 3))
          {
            // Envia ACK de confirmação.
            mdb_envia(0x000);
            // Pega o valor do tubo.
            valor = verifica_coin_tybe_deposited(mensagem);   
            // Incrementa uma moeda.          
            update_cofre(valor); 
            // Faz o calculo e transforma no valor em centavos da moeda.
            valor_depositado = (info_coin.tipo_credito[valor]*info_coin.fator_escala);
            // Altera o estado da aceitação para CASH_BOX.
            estado_aceitacao = CASH_BOX; 
            
            // Serial de debug 
            #ifdef DEBUG_ACEITACAO 
            Serial.print(F("VALOR INSERIDO COFRE: "));
            Serial.println(valor_depositado); 
            #endif  
          }                              
          break;
     //Moeda depositada nos Tubos.     
     case TUBES:
          if(confirma_chk(mensagem, 3))
          {
            // Envia ACK de confirmação.
            mdb_envia(0x000);
            // Pega o valor do tubo.
            valor = verifica_coin_tybe_deposited(mensagem);   
            // Incrementa uma moeda no tubo.         
            update_TUBEStatus(valor);
            // Faz o calculo e transforma no valor em centavos da moeda.
            valor_depositado = (info_coin.tipo_credito[valor]*info_coin.fator_escala);
            // Altera o estado da aceitação para TUBES.
            estado_aceitacao = TUBES; 
            
            // Serial de debug 
            #ifdef DEBUG_ACEITACAO 
            Serial.print(F("VALOR INSERIDO TUBO: "));
            Serial.println(valor_depositado); 
            #endif  
          }
          break;
     //Nao utilizada.     
     case NOT_USED:
          // Envia ACK de confirmação.
          mdb_envia(0x000);
          break;
     //Moeda rejeitada/desconhecida.     
     case REJECT:
          // Envia ACK de confirmação.
          mdb_envia(0x000);
          break;     
  } 
}

/*********************************************************************************************************
** Nome da Função:      is_bill_jof
** Descrição:           Verifica se o equipamento noteiro é um ITL.
** Parametro:           Não
** Valor de retorno:    True or False.
*********************************************************************************************************/
bool MDB::is_bill_jof()
{
  // Verifica o codigo do fabricante.
  for(int i =0; i<3;i++)
  {
     if ( info_bill.manufacturer_code[i] == 'J' )
     {
       return true;
     }
  } 
  return false;
}

/*********************************************************************************************************
** Nome da Função:      deposito_bill
** Descrição:           Funcao que recebe o valor de deposito bill do protocolo MDB 
** Parametro:           mensagem: valor vindo do mdb.
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::deposito_bill(int mensagem)
{
  int bill_deposited;
  int valor;
  
  //Chama a funcao que verifica o status da nota inserida.
  bill_deposited = verifica_bill_routing(mensagem);

  switch(bill_deposited)
  {
    //Nota foi para o armazenamento.
    case BILL_STACKED:
          if ( !is_itl() && !is_bill_jof() )
          {
            if(confirma_chk(mensagem, 3))
            {
              // Envia ACK de confirmação.
              mdb_envia(0x000);
              // Pega o tipo da nota.
              valor = verifica_bill_tybe_deposited(mensagem);              
                         
              // Faz o calculo e transforma no valor em centavos da nota.               
              valor_depositado = (info_bill.tipo_credito[valor]*info_bill.fator_escala);            
            
              // Se a nota estava no estado escrow.
              if( nota_em_custodia == 0 )    
              {              
                // Altera o estado da aceitação para BILL_STACKED.
                estado_aceitacao = BILL_STACKED;
                
                // Serial de debug 
                #ifdef DEBUG_ACEITACAO 
                Serial.print(F("VALOR INSERIDO NOTA(ESCROW INATIVO): "));
                Serial.println(valor_depositado);
                #endif 
              }else{
                      // Altera o estado da aceitação para ESCROW_STACKED.
                      estado_aceitacao = ESCROW_STACKED;
                      // Seta a variavel de controle da nota em custodia em 0.
                      nota_em_custodia = 0;
                      
                      // Serial de debug 
                      #ifdef DEBUG_ACEITACAO 
                      Serial.print(F("VALOR INSERIDO DO ESCROW PARA STACKED: "));
                      Serial.println(valor_depositado);
                      #endif 
                   }                           
            } 
          }else if ( is_itl() )
          {
            if(confirma_chk(mensagem, 2))
            {
              // Envia ACK de confirmação.
              mdb_envia(0x000);
              // Pega o tipo da nota.
              valor = verifica_bill_tybe_deposited(mensagem);              
                         
              // Faz o calculo e transforma no valor em centavos da nota.               
              valor_depositado = (info_bill.tipo_credito[valor]*info_bill.fator_escala);            
            
              // Se a nota estava no estado escrow.
              if( nota_em_custodia == 0 )    
              {              
                // Altera o estado da aceitação para BILL_STACKED.
                estado_aceitacao = BILL_STACKED;
                
                // Serial de debug 
                #ifdef DEBUG_ACEITACAO 
                Serial.print(F("VALOR INSERIDO NOTA(ESCROW INATIVO): "));
                Serial.println(valor_depositado);
                #endif 
              }else{
                      // Altera o estado da aceitação para ESCROW_STACKED.
                      estado_aceitacao = ESCROW_STACKED;
                      // Seta a variavel de controle da nota em custodia em 0.
                      nota_em_custodia = 0;
                      
                      // Serial de debug 
                      #ifdef DEBUG_ACEITACAO 
                      Serial.print(F("VALOR INSERIDO DO ESCROW PARA STACKED: "));
                      Serial.println(valor_depositado);
                      #endif 
                   }                           
            }
          }else
          {
            mdb_recebe();
            // Envia ACK de confirmação.
            mdb_envia(0x000);
            // Pega o tipo da nota.
            valor = verifica_bill_tybe_deposited(mensagem);              
                       
            // Faz o calculo e transforma no valor em centavos da nota.               
            valor_depositado = (info_bill.tipo_credito[valor]*info_bill.fator_escala);            
          
            // Se a nota estava no estado escrow.
            if( nota_em_custodia == 0 )    
            {              
              // Altera o estado da aceitação para BILL_STACKED.
              estado_aceitacao = BILL_STACKED;
              
              // Serial de debug 
              #ifdef DEBUG_ACEITACAO 
              Serial.print(F("VALOR INSERIDO NOTA(ESCROW INATIVO): "));
              Serial.println(valor_depositado);
              #endif 
             }else{
                    // Altera o estado da aceitação para ESCROW_STACKED.
                    estado_aceitacao = ESCROW_STACKED;
                    // Seta a variavel de controle da nota em custodia em 0.
                    nota_em_custodia = 0;
                    
                    // Serial de debug 
                    #ifdef DEBUG_ACEITACAO 
                    Serial.print(F("VALOR INSERIDO DO ESCROW PARA STACKED: "));
                    Serial.println(valor_depositado);
                    #endif 
                   }  
          }          
          break;
    //Nota em posicao de ESCROW.(Retida ate finalizacao da compra).                  
    case ESCROW_POSITION:
          if ( !is_itl() )
          {
            if(confirma_chk(mensagem, 3))
            {  
              // Envia ACK de confirmação.
              mdb_envia(0x000);
              // Pega o tipo da nota.
              valor = verifica_bill_tybe_deposited(mensagem);
              // Faz o calculo e transforma no valor em centavos da nota. 
              valor_depositado = (info_bill.tipo_credito[valor]*info_bill.fator_escala);
              // Altera o estado da aceitação para ESCROW_POSITION.
              estado_aceitacao = ESCROW_POSITION; 
              // Verifica se é uma dispensa de moeda(mesmo codigo MDB mas o valor inserido é 0).
              if ( valor_depositado != 0 )
              {
                // Serial de debug 
                #ifdef DEBUG_ACEITACAO 
                Serial.print(F("VALOR INSERIDO NOTA(ESCROW ATIVO): "));
                Serial.println(valor_depositado);
                #endif 
              }
            } 
          }else
          {
            if(confirma_chk(mensagem, 2))
            {  
              // Envia ACK de confirmação.
              mdb_envia(0x000);
              // Pega o tipo da nota.
              valor = verifica_bill_tybe_deposited(mensagem);
              // Faz o calculo e transforma no valor em centavos da nota. 
              valor_depositado = (info_bill.tipo_credito[valor]*info_bill.fator_escala);
              // Altera o estado da aceitação para ESCROW_POSITION.
              estado_aceitacao = ESCROW_POSITION; 
              // Verifica se é uma dispensa de moeda(mesmo codigo MDB mas o valor inserido é 0).
              if ( valor_depositado != 0 )
              {
                // Serial de debug 
                #ifdef DEBUG_ACEITACAO 
                Serial.print(F("VALOR INSERIDO NOTA(ESCROW ATIVO): "));
                Serial.println(valor_depositado);
                #endif 
              }
            } 
          }
          break;
    //Nota inserida errada ou desconhecida.                  
    case BILL_RETURNED:
          if ( !is_itl() )
          {
            if(confirma_chk(mensagem, 3))
            {  
              // Envia ACK de confirmação.
              mdb_envia(0x000);
              // Pega o tipo da nota.
              valor = verifica_bill_tybe_deposited(mensagem);
              
              // Serial de debug 
              #ifdef DEBUG_ACEITACAO 
              Serial.print(F("VALOR RETORNADO: "));
              Serial.println(((info_bill.tipo_credito[valor]*info_bill.fator_escala)));
              #endif 
            }
          }else
          {
            if(confirma_chk(mensagem, 2))
            {  
              // Envia ACK de confirmação.
              mdb_envia(0x000);
              // Pega o tipo da nota.
              valor = verifica_bill_tybe_deposited(mensagem);
              
              // Serial de debug 
              #ifdef DEBUG_ACEITACAO 
              Serial.print(F("VALOR RETORNADO: "));
              Serial.println(((info_bill.tipo_credito[valor]*info_bill.fator_escala)));
              #endif 
            }
          }
          break;
    //Reciclador(Nivel 3+).                  
    case BILL_TO_RECYCLER:
          // Envia ACK de confirmação.
          mdb_envia(0x000);
          break;
    //Nota rejeitada.                  
    case DISABLE_BILL_REJECTED:
          // Envia ACK de confirmação.
          mdb_envia(0x000);
          break;
    //Reciclador(MANUAL-FILL)                  
    case BILL_TO_RECYCLER_MANUAL:
          // Envia ACK de confirmação.
          mdb_envia(0x000);
          break;
    //Dispensa manual                  
    case MANUAL_DISPENSE:
          // Envia ACK de confirmação.
          mdb_envia(0x000);
          break;
    case TRANSF_FROM_RECYCLER:
          // Envia ACK de confirmação.
          mdb_envia(0x000);
          break;                  
  }
}

/*********************************************************************************************************
** Nome da Função:      calc_bill_type
** Descrição:           Funcao que verifica, quais notas devem ficar "Ativas" , faz o calculo e gera 
*                       os valores para o codigo BILL TYPE. 
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::calc_bill_type()
{
  int y_1 = 0;
  int y_2 = 0;
  int mlt = 1;
  
  for(int i=0; i<16; i++)
  {
    if(i<8)
    {
      if(info_bill.status_ativo[i]==ATIVO)
      {
        y_1 = y_1 + mlt;
      }
    }else if(info_bill.status_ativo[i]==ATIVO){
            y_2 = y_2 + mlt;
          }     
    mlt = mlt*2;    
    if(i==7)
    {
      mlt = 1;
    }   
  }
  //Chama a funcao que envia o codigo MDB.
  bill_type(y_2,y_1);
}  

/*********************************************************************************************************
** Nome da Função:      bill_type
** Descrição:           Funcao que envia o BILL TYPE, com dados especificos de notas a serem aceitas.
** Parametro:           y_1 primeiro byte de notas aceitas, y_2 segundo byte de notas aceitas.
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::bill_type(int y_1, int y_2)
{
  int data[20];
  data[0] = 0x134;          
  data[1] = y_1;
  data[2] = y_2;  
  data[3] = type_escrow_1;         
  data[4] = type_escrow_2;
  //Calculando CHK necessario.
  data[5] = calc_chk(data, 6);
  for (int cont = 0; cont < 6; cont++)
  {
    mdb_envia(data[cont]);      //Enviando os dados.
  }
  Serial1.read();
}  

/*********************************************************************************************************
** Nome da Função:      aceitacao
** Descrição:           Tarefa que recebe os 2 bytes de notas ativas.
** Parametro:           notas_ativa[]: vetor com 2 bytes de valores para notas ativas.
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::aceitacao(int notas_ativa[], int moedas_ativa[])
{
  int contador_m = 0;
  int bit_m = 1;
  
   for(int i=0;i<2;i++)
  {
    for(int i_2=0; i_2<8; i_2++)
    {
      
      if((notas_ativa[i] & bit_m) == bit_m)
      {
        info_bill.status_ativo[contador_m] = ATIVO;
      }else{
              info_bill.status_ativo[contador_m] = INATIVO;
            } 
      if(( moedas_ativa[i] & bit_m) == bit_m)
      {
        info_coin.status_ativo[contador_m] = ATIVO;
      }else{
              info_coin.status_ativo[contador_m] = INATIVO;
            }       
      if(bit_m==128)
      {
        bit_m = 1;
      }else{
        bit_m = bit_m*2;
      }
      contador_m++;
    }
  }  
}

/*********************************************************************************************************
** Nome da Função:      dispense_recycle
** Descrição:           Envia o codigo MDB para dispensa no reciclador de notas.
** Parametro:           Valor a dispensar.
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::dispense_recycle(int valor)
{
  valor = valor * disp_reciclador;
  data[0] = 0x137;         
  data[1] = 0x007;
  data[2] = 0x000;  
  data[3] = valor;  
  data[4] = calc_chk(data,5) ;                    
  for(int cont=0; cont < 5; cont++)
  {
    mdb_envia(data[cont]);      //Enviando os dados.
  }
  delay(15);
  if( mdb_recebe() == 0x100 )
  {
     payout_status = 4; 
  }
}

/*********************************************************************************************************
** Nome da Função:      dispensa_reciclador
** Descrição:           Faz a dispensa de notas no equipamento reciclador de notas.
** Parametro:           Valor a dispensar.
** Valor de retorno:    Valor dispensado
*********************************************************************************************************/
int MDB::dispensa_reciclador(int valor_troco)
{
   int aux = 0;
   
   // Descarta possiveis dados vindo do poller, para fazer a entrega pelo reciclador.
   descarta_leitura(5);
   // Seta o boot em 0 para para a leitura/envio do poller.
   boot_mdb=0;
   // Envia comando para verificar a quantidade de notas no reciclador.
   mdb_envia(0x137);
   mdb_envia(0x005);
   mdb_envia(0x03C);
   // Aguarda os bytes chegarem na serial1.
   while(Serial1.available()<30)
   {
   }       
   // Faz a leitura dos dados do mdb.
   for(int i = 0; i < 35; i++)
   {
     if(Serial1.available())
     {
       data[i] = Serial1.read();
       delay(15); 
       #ifdef DEBUG_MDB_DETAIL
       Serial.print(F(" Resposta["));
       Serial.print(i);
       Serial.print(F("]:"));
       Serial.println(data[i]);
       #endif           
     }            
   } 
   // Retorna a atividade do poller do mdb.
   boot_mdb = 1;    
   // Valida o chk.   
   if( validar_chk(data, 35) ) 
   {
      //Checksum ok, manda ACK e proseguir
      mdb_envia(0x000);
      // Armazena a quantidade de notas no reciclador.
      qtd_notas_disp = data[3];
   }else {
            // Zera a quantidade de notas a disponiveis para dispensa.
            qtd_notas_disp = 0;
         }
  // Verifica se o valor do troco é superior a o valor da nota que esta sendo reciclada e se a quantidade de nota a dispensar é maior que 0.
  if( valor_troco > disp_reciclador && qtd_notas_disp > 0 )
  {
    // Divide o troco pelo valor da nota que é reciclada( exemplo: troco que deve ser dado é 800 e o valor da nota que sai do reciclador é 200, entao 800/200 = 4).
    aux = valor_troco / disp_reciclador;
    // Verifica se o valor da divisao for maior que a quantidade de notas a disponiveis, dispensa as notas disponiveis e retorna o valor da diferença.
    if( aux > qtd_notas_disp )
    {
      // Envia comando mdb para dispensa das notas disponiveis.
      dispense_recycle(qtd_notas_disp);  
      
      #ifdef DEBUG_ACEITACAO
      Serial.print(F("VALOR DISPENSADO RECICLADOR: "));
      Serial.println(qtd_notas_disp*disp_reciclador);
      #endif
      // Retorna o valor dispensado.
      return (qtd_notas_disp*disp_reciclador);
      
    }else{
            // Envia comando mdb para dispensa das notas disponiveis.
            dispense_recycle(aux);
            
            #ifdef DEBUG_ACEITACAO
            Serial.print(F("VALOR DISPENSADO RECICLADOR: "));
            Serial.println(aux*disp_reciclador);
            #endif
            // Retorna o valor dispensado.
            return (aux*disp_reciclador);
          }                     
  }
  
  
  
}

/*********************************************************************************************************
** Nome da Função:      set_type_escrow
** Descrição:           Altera valor do comando type escrow
** Parametro:           Valor para alterar.
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::set_type_escrow(int type_1, int type_2)
{
    type_escrow_1 = type_1;
    type_escrow_2 = type_2;
}

/*********************************************************************************************************
** Nome da Função:     	get_coin_coin_ativo
** Descrição:           Retorna o valor da variavel.
** Parametro:           Não
** Valor de retorno:    Retorna o valor da variavel coin_coin_ativo.
*********************************************************************************************************/
int MDB::get_coin_coin_ativo()
{
    return coin_coin_ativo;
}

/*********************************************************************************************************
** Nome da Função:      set_coin_coin_ativo
** Descrição:           Altera valor da variavel coin_coin_ativo
** Parametro:           Valor para alterar.
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::set_coin_coin_ativo(int x)
{
    coin_coin_ativo = x;
}

/*********************************************************************************************************
** Nome da Função:     	get_bill_disable
** Descrição:           Retorna o valor da variavel.
** Parametro:           Não
** Valor de retorno:    Retorna o valor da variavel bill_disable.
*********************************************************************************************************/
int MDB::get_bill_disable()
{
    return bill_disable;
}

/*********************************************************************************************************
** Nome da Função:      set_bill_disable
** Descrição:           Altera valor da variavel bill_disable
** Parametro:           Valor para alterar.
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::set_bill_disable(int x)
{
   bill_disable = x;
}

/*********************************************************************************************************
** Nome da Função:     	get_valor_depositado
** Descrição:           Retorna o valor da variavel.
** Parametro:           Não
** Valor de retorno:    Retorna o valor da variavel coin_coin_ativo.
*********************************************************************************************************/
int MDB::get_valor_depositado()
{
    return valor_depositado;
}

/*********************************************************************************************************
** Nome da Função:      set_valor_depositado
** Descrição:           Altera valor da variavel valor_depositado
** Parametro:           Valor para alterar.
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::set_valor_depositado(int x)
{
    valor_depositado = x;
}

/*********************************************************************************************************
** Nome da Função:     	get_estado_aceitacao
** Descrição:           Retorna o valor da variavel.
** Parametro:           Não
** Valor de retorno:    Retorna o valor da variavel estado_aceitacao.
*********************************************************************************************************/
int MDB::get_estado_aceitacao()
{
    return estado_aceitacao;
}

/*********************************************************************************************************
** Nome da Função:     	get_inicio_sessao
** Descrição:           Retorna o valor da variavel.
** Parametro:           Não
** Valor de retorno:    Retorna o valor da variavel inicio_sessao.
*********************************************************************************************************/
int MDB::get_inicio_sessao()
{
    return inicio_sessao;
}

/*********************************************************************************************************
** Nome da Função:      set_inicio_sessao
** Descrição:           Altera valor da variavel inicio_sessao
** Parametro:           Valor para alterar.
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::set_inicio_sessao(int x)
{
    inicio_sessao = x;
}

/*********************************************************************************************************
** Nome da Função:     	get_venda_aprovada
** Descrição:           Retorna o valor da variavel.
** Parametro:           Não
** Valor de retorno:    Retorna o valor da variavel venda_aprovada.
*********************************************************************************************************/
int MDB::get_venda_aprovada()
{
    return venda_aprovada;
}

/*********************************************************************************************************
** Nome da Função:      set_venda_aprovada
** Descrição:           Altera valor da variavel venda_aprovada
** Parametro:           Valor para alterar.
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::set_venda_aprovada(int x)
{
    venda_aprovada = x;
}

/*********************************************************************************************************
** Nome da Função:     	get_venda_negada
** Descrição:           Retorna o valor da variavel.
** Parametro:           Não
** Valor de retorno:    Retorna o valor da variavel venda_negada.
*********************************************************************************************************/
int MDB::get_venda_negada()
{
    return venda_negada;
}

/*********************************************************************************************************
** Nome da Função:      set_venda_negada
** Descrição:           Altera valor da variavel venda_negada
** Parametro:           Valor para alterar.
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::set_venda_negada(int x)
{
    venda_negada = x;
}

/*********************************************************************************************************
** Nome da Função:     	get_sessao_finalizada
** Descrição:           Retorna o valor da variavel.
** Parametro:           Não
** Valor de retorno:    Retorna o valor da variavel sessao_finalizada.
*********************************************************************************************************/
int MDB::get_sessao_finalizada()
{
    return sessao_finalizada;
}

/*********************************************************************************************************
** Nome da Função:      set_sessao_finalizada
** Descrição:           Altera valor da variavel sessao_finalizada
** Parametro:           Valor para alterar.
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::set_sessao_finalizada(int x)
{
    sessao_finalizada = x;
}

/*********************************************************************************************************
** Nome da Função:      set_boot_mdb
** Descrição:           Altera valor da variavel boot_mdb
** Parametro:           Valor para alterar.
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::set_boot_mdb(int x)
{
   boot_mdb = x;
}

/*********************************************************************************************************
** Nome da Função:      set_disp_reciclador
** Descrição:           Altera valor da variavel disp_reciclador
** Parametro:           Valor para alterar.
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::set_disp_reciclador(int x)
{
   disp_reciclador = x;
}

/*********************************************************************************************************
** Nome da Função:      verifica_reciclador
** Descrição:           Verifica se o equipamento bill é um reciclador.
** Parametro:           Não
** Valor de retorno:    1 Sim, 2 Não
*********************************************************************************************************/
bool MDB::verifica_reciclador()
{
   if( info_bill.feature_level >= 2 && itl == 0 )
    return true;
   else
    return false;     
}

/*********************************************************************************************************
** Nome da Função:      coin_esta_ativo
** Descrição:           Verifica se o equipamento coin esta ativo.
** Parametro:           Não
** Valor de retorno:    1 Sim, 2 Não
*********************************************************************************************************/
bool MDB::coin_esta_ativo()
{
   if( boot_coin == 1 )
    return true;
   else
    return false;     
}

/*********************************************************************************************************
** Nome da Função:      bill_esta_ativo
** Descrição:           Verifica se o equipamento bill esta ativo.
** Parametro:           Não
** Valor de retorno:    1 Sim, 2 Não
*********************************************************************************************************/
bool MDB::bill_esta_ativo()
{
   if( boot_bill == 1 )
    return true;
   else
    return false;     
}

/*********************************************************************************************************
** Nome da Função:      cash_esta_ativo
** Descrição:           Verifica se o equipamento cash esta ativo.
** Parametro:           Não
** Valor de retorno:    1 Sim, 2 Não
*********************************************************************************************************/
bool MDB::cash_esta_ativo()
{
   if( boot_cash == 1 )
    return true;
   else
    return false;     
}

/*********************************************************************************************************
** Nome da Função:      confirma_chk
** Descrição:           Função que coloca em um vetor os dados e verifica o chk.
** Parametro:           Mensagem: primeira mensagem, qtd_msg: quantidade de mensagens.
** Valor de retorno:    1: Sim, 2: Não.
*********************************************************************************************************/
bool MDB::confirma_chk(int mensagem, int qtd_msg)
{
  int vetor_msg[10];
  bool validacao;
  
  vetor_msg[0] = mensagem;
  
  for(int i=1; i<qtd_msg; i++)
  {
    delay(15);
    vetor_msg[i] = mdb_recebe();    
  }
  
  if(mensagem == VEND_APPROVED)
  {
    data[0] = vetor_msg[1];
    data[1] = vetor_msg[2];
  }
  
  validacao = validar_chk(vetor_msg, qtd_msg);
    
  return validacao;
  
}

/*********************************************************************************************************
** Nome da Função:      status_coin
** Descrição:           Função que verifica alguma alteração de status do coin recebido pelo poll.
** Parametro:           Mensagem: mensagem do mdb.
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::status_coin(int mensagem)
{
  // Switch para ações do equipamento aceitador de moedas(COIN).
  switch(mensagem)
  {
    // Alavanca do coin coi acionada.
    case ALAVANCA_COIN_COIN:
        if(confirma_chk(ALAVANCA_COIN_COIN, 2))
        {
          // Serial de debug 
          #ifdef DEBUG_ACEITACAO 
          Serial.println(F("COIN COIN ATIVADO."));
          #endif
          // Envia ACK de confirmação.
          mdb_envia(0x000);
          // Seta a variavel de controle coin_coin_ativo para ATIVO.
          coin_coin_ativo = ATIVO;
        }
        break; 
    /* // Houve um erro na formulação do chk.
     case CHK_ERROR:
        if(confirma_chk(CHK_ERROR, 2))
        {
          // Serial de debug 
          #ifdef DEBUG_ACEITACAO 
          Serial.println(F("ERRO AO FORMULAR CHK."));
          #endif
          // Envia ACK de confirmação.
          mdb_envia(0x000);
        }
        break;  */
     // Moeda encravada no caminho da aceitação.
     case COIN_JAM:
        if(confirma_chk(COIN_JAM, 2))
        {
          // Serial de debug 
          #ifdef DEBUG_ACEITACAO 
          Serial.println(F("MOEDA ENCRAVADA."));
          #endif
          // Envia ACK de confirmação.
          mdb_envia(0x000);
        }
        break; 
     // Houve uma tentativa de retirar a moeda no momento da aceitação.
     case COIN_REMOVAL:
        if(confirma_chk(COIN_REMOVAL, 2))
        {
          // Serial de debug 
          #ifdef DEBUG_ACEITACAO 
          Serial.println(F("TENTATIVA DE RETIRAR A MOEDA NO MOMENTO DA ACEITACAO."));
          #endif
          // Envia ACK de confirmação.
          mdb_envia(0x000);
        }
        break;    
  }
}

/*********************************************************************************************************
** Nome da Função:      status_bill
** Descrição:           Função que verifica alguma alteração de status do bill recebido pelo poll.
** Parametro:           Mensagem: mensagem do mdb.
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::status_bill(int mensagem)
{
  // Switch para ações do equipamento aceitador de notas(BILL).
  switch(mensagem)
  {     
    // Bill Validador.
    // Nota rejeitada pelo equipamento leitor de notas.    
    case 6:
         if ( bill_was_reset == 0 )
         {
           time_start_bill_rst = millis();
         }
         tempo_atual_bill_rst = millis();         
         if((tempo_atual_bill_rst-time_start_bill_rst) > 5000)
         {
           bill_was_reset = 0; 
         }
         bill_was_reset++;
         if ( bill_was_reset == 5 )
         {
           // Serial de debug 
           #ifdef DEBUG_ACEITACAO 
           Serial.println(F("BILL RESET."));
           #endif
           // Envia ACK de confirmação.
           mdb_envia(0x000);
           boot_mdb = 0;
           boot_bill = 0;
           controle_bill = 2;
           bill_was_reset = 0;
        }
        break;
    // Cashbox out of position   
    case 8:
        // Serial de debug 
        #ifdef DEBUG_ACEITACAO 
        Serial.println(F("CASH BOX OUT OF POSITION."));
        #endif
        // Envia ACK de confirmação.
        mdb_envia(0x000);
        
        inicia_notas(0,1,1);
        break;   
    // Leitor disabilitado.
    case 9:
        if ( bill_was_disable == 0 )
         {
           time_start_disable = millis();
         }
         tempo_atual_disable = millis();         
         if((tempo_atual_disable-time_start_disable) > 5000)
         {
           bill_was_disable = 0; 
         }
         bill_was_disable++;
         if ( bill_was_disable == 5 )
         {
           // Serial de debug 
           #ifdef DEBUG_ACEITACAO 
           Serial.println(F("BILL DISABLE."));
           #endif
           // Envia ACK de confirmação.
           mdb_envia(0x000);
           bill_disable = 1;
        } 
        break;      
    // Nota rejeitada pelo equipamento leitor de notas.    
    case BILL_REJECTED:
        if(confirma_chk(BILL_REJECTED, 2))
        {
          // Serial de debug 
          #ifdef DEBUG_ACEITACAO 
          Serial.println(F("NOTA REJEITADA."));
          #endif
          // Envia ACK de confirmação.
          mdb_envia(0x000);
        }
        break; 
    // Bill Reciclador.  
    // Pedido de custodia foi detectado. 
    case BILL_RECICLADOR_ESCROW_REQUEST:
        if(confirma_chk(BILL_RECICLADOR_ESCROW_REQUEST, 2))
        {
          // Serial de debug 
          #ifdef DEBUG_ACEITACAO 
          //Serial.println(F("PEDIDO DE CUSTODIA RECICLADOR."));
          #endif
          // Envia ACK de confirmação.
          mdb_envia(0x000);
        }
        break;   
    // Chave de preenchimento ativada.
    case BILL_RECICLADOR_FILLED_KEY:
        if(confirma_chk(BILL_RECICLADOR_FILLED_KEY, 2))
        {
          // Serial de debug 
          #ifdef DEBUG_ACEITACAO 
          Serial.println(F("CHAVE DE PREENCHIMENTO ATIVADA."));
          #endif
          // Envia ACK de confirmação.
          mdb_envia(0x000);
        }
        break;    
    case 34:
        if(confirma_chk(34, 2))
        {
          // Envia ACK de confirmação.
          mdb_envia(0x000);
        }
        break;   
    case 35:
        if(confirma_chk(35, 2))
        {
          // Envia ACK de confirmação.
          mdb_envia(0x000);
        }
        break;     
  }
}

/*********************************************************************************************************
** Nome da Função:      status_cash
** Descrição:           Função que verifica alguma alteração de status do cash recebido pelo poll.
** Parametro:           Mensagem: mensagem do mdb.
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::status_cash(int mensagem)
{  
  // Switch para ações do equipamento aceitador de notas(BILL).
  switch(mensagem)
  { 
     // Inicio de sessão.
    case JUST_RESET:
        delay(15);
        mdb_recebe();
        if ( payout_status == 0 )
        {
          if ( just_was_reset == 0 )
          {
            time_start_just_rst = millis();
          }
          tempo_atual_just_rst = millis();         
          if((tempo_atual_just_rst-time_start_bill_rst) > 5000)
          {
            just_was_reset = 0; 
          }
          just_was_reset++;
          if ( just_was_reset == 10 )
          {          
            // Serial de debug 
            #ifdef DEBUG_ACEITACAO 
            Serial.println(F("JUST RESET."));
            #endif
            mdb_envia(0x000);       
            // Reseta o boot dos equipamentos mdb.
            controle_bill = 2;
            controle_coin = 2;
            controle_cash = 2; 
            boot_mdb = 0;
            inicio_sessao = INATIVO;
            // Controle de estados do poller.
            mdb_task_ctl = 0;
          }
        }
        break; 
    // Inicio de sessão.
    case BEGIN_SESSION:
        if(confirma_chk(BEGIN_SESSION, 4))
        {
          // Serial de debug 
          #ifdef DEBUG_ACEITACAO 
          Serial.println(F("INICIO DE SESSAO CASHLESS."));
          #endif
          mdb_envia(0x000);       
          inicio_sessao = 1;   
        }
        break;
    // Compra aprovada.
    case VEND_APPROVED:
        if(confirma_chk(VEND_APPROVED, 4))
        {
          // Serial de debug 
          #ifdef DEBUG_ACEITACAO 
          Serial.print(F("VENDA APROVADA CASHLESS: "));
          Serial.print((data[0]*256)+data[1]);
          Serial.println(F(" CENTAVOS"));
          #endif
          mdb_envia(0x000);        
          venda_aprovada = 1;
        }
        break; 
    // Solicitação para finlizar a sessão.
    case SESSION_CANCEL:
        if(confirma_chk(SESSION_CANCEL, 2))
        {
          // Serial de debug 
          #ifdef DEBUG_ACEITACAO 
          Serial.println(F("SOLICITACAO PARA FINALIZAR SESSAO."));
          #endif
          mdb_envia(0x000);   
          session_complete();     
        }
        break;
    // Sessão finalizada.
    case END_SESSION:
        if(confirma_chk(END_SESSION, 2))
        {
          // Serial de debug 
          #ifdef DEBUG_ACEITACAO 
          Serial.println(F("SESSAO FINALIZADA."));
          #endif
          mdb_envia(0x000);      
          sessao_finalizada = 1;
        }
        break; 
    // Venda negada.
    case VEND_DENIED:
        if(confirma_chk(VEND_DENIED, 2))
        {
          // Serial de debug 
          #ifdef DEBUG_ACEITACAO 
          Serial.println(F("VENDA NEGADA."));
          #endif
          mdb_envia(0x000); 
          venda_negada = 1;     
        }
        break;     
  }
}

/*********************************************************************************************************
** Nome da Função:      informacoes_coin
** Descrição:           Método que retorna a estrutura com os dados do equipamento coin
** Parametro:           Não
** Valor de retorno:    Estrutura INFO_COIN
*********************************************************************************************************/
INFO_COIN MDB::informacoes_coin()
{
  return info_coin;
}

/*********************************************************************************************************
** Nome da Função:      informacoes_bill
** Descrição:           Método que retorna a estrutura com os dados do equipamento bill
** Parametro:           Não
** Valor de retorno:    Estrutura INFO_BILL
*********************************************************************************************************/
INFO_BILL MDB::informacoes_bill()
{
  return info_bill;
}

/*********************************************************************************************************
** Nome da Função:      informacoes_cash
** Descrição:           Método que retorna a estrutura com os dados do equipamento cashless
** Parametro:           Não
** Valor de retorno:    Estrutura INFO_CASH
*********************************************************************************************************/
INFO_CASH MDB::informacoes_cash()
{
  return info_cash;
}

/*********************************************************************************************************
** Nome da Função:      tubos_coin
** Descrição:           Método que retorna a estrutura com os dados referentes aos tubos do aceitador de 
                        moedas.
** Parametro:           Não
** Valor de retorno:    Estrutura STATUS_TUBO
*********************************************************************************************************/
STATUS_TUBO MDB::tubos_coin()
{
  return Status_tubo;
}

/*********************************************************************************************************
** Nome da Função:      entregar_troco
** Descrição:           Método que solicita ao moedeiro o troco desejado.
** Parametro:           Valor a ser entregue de troco.
** Valor de retorno:    Não
*********************************************************************************************************/
int MDB::entregar_troco(int valor)
{
  valor = valor/5;
  data[0] = 0x10F;         
  data[1] = 0x002;
  data[2] = valor;  
  data[3] = calc_chk(data,4) ;                    
  for(int cont=0; cont < 4; cont++)
  {
    mdb_envia(data[cont]);      //Enviando os dados.
  }
  delay(15);
  if( mdb_recebe() == 0x100 )
  {
     payout_status = 1; 
  }
}  

/*********************************************************************************************************
** Nome da Função:      pay_out
** Descrição:           Tarefa que verifica a entrega do troco pelo comando pay out.
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::pay_out()
{  
  int mensagem;
  
  tempo_atual_verifica_payout = millis();
  if((tempo_atual_verifica_payout-time_start_verifica_payout) > 60)
  {
    time_start_verifica_payout = millis();
    switch(payout_status)
    {
      case 1:
          data[0] = 0x10F;         
          data[1] = 0x004;
          data[2] = calc_chk(data,3) ;                    
          for(int cont=0; cont < 3; cont++)
          {
            mdb_envia(data[cont]);      //Enviando os dados.
          }  
          delay(15);       
          mensagem = mdb_recebe();
          if( mensagem != 0 )
          {            
            if( mensagem == 0x100 )
            {
              payout_status++;
            }else{
                    mdb_envia(0x000);
                 }
          }          
          break;
      //Enviando comando TUBE STATUS.   
      case 2:      
        mdb_envia(0x10A);
        mdb_envia(0x00A);
        payout_status++;
        break;
      //Esperar respostas.  
      case 3:    
         if(Serial1.available() < 18) 
         {
           return;
         }  
         //Ler os 18 dados e o CHK.
         for(int i = 0; i < 19; i++)
         {
            data[i] = Serial1.read();
            #ifdef DEBUG_BOOT_MDB
            Serial.print(F("Dados TUBE STATUS: "));
            Serial.println(data[i]);
            #endif
         }
         Status_tubo.cheio = (data[0] << 8 | data[1]);
         for(int i=0; i < 19; i++) 
         {
           Status_tubo.Status[i] = data[i+2];
         }
         payout_status = 0;
         break;        
      case 4:
          data[0] = 0x137;         
          data[1] = 0x009;
          data[2] = calc_chk(data,3) ;                    
          for(int cont=0; cont < 3; cont++)
          {
            mdb_envia(data[cont]);      //Enviando os dados.
          }  
          delay(15);       
          mensagem = mdb_recebe();
          if( mensagem != 0 )
          {            
            if( mensagem == 0x100 )
            {
              payout_status = 0;
            }else{
                    mdb_envia(0x000);
                 }
          }          
          break;    
    }
  }
}

/*********************************************************************************************************
** Nome da Função:      mdb_leitura
** Descrição:           Tarefa que le o retorno do poll dos equipamentos MDB.
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::mdb_leitura()
{
  int destino_mdb=0;
  int mensagem;
  
  tempo_atual_mdb= millis();
  if((tempo_atual_mdb-time_start_mdb) > 7)
  {
     if ( mdb_avail() )
     {
        // Variavel que controla se o mdb esta comunicando ou não.
        sem_retorno_mdb = INATIVO;
        
        // Recebe o valor do mdb na variavel mensagem.
        mensagem = mdb_recebe();
        
        //Visualizacao do dado recebido.
        #ifdef DEBUG_MDB
        Serial.print(F(" Resposta : "));
        Serial.println(mensagem);
        #endif
        
        /*if( mensagem != 256 )
        {
          Serial.print(F(" Resposta : "));
          Serial.println(mensagem);
        }*/
        
        // Verifica os bits para ver se é um deposito coin ou deposito bill.
        destino_mdb = verifica_aceitacao(mensagem);                
              
        // Switch para deposito de moeda e nota.        
        switch(destino_mdb)
        {
          // Se deposito coin.
          case COIN_DEPOSITADO:
            deposito_coin(mensagem);
            break;
          // Se deposito bill.
          case BILL_DEPOSITADO:
            tempo_atual_billrt = millis();
            if((tempo_atual_billrt-time_start_billrt) > 300)
            {  
              deposito_bill(mensagem);
            }
            mdb_envia(0x000);
            time_start_billrt = millis();
            break;  
        }
        
        //Função que verifica açoes realizadas referentes ao equipamento aceitador de moedas(dados recebidos do poll).
        //status_coin(mensagem);
        
        //Função que verifica açoes realizadas referentes ao equipamento aceitador de cedulas(dados recebidos do poll).
        status_bill(mensagem);
        
        //Função que verifica açoes realizadas referentes ao equipamento de cobrança por cartão(dados recebidos do poll).
       // status_cash(mensagem);        
        
     }else
     {
       // Variavel que controla se o mdb esta comunicando ou não.
        sem_retorno_mdb = ATIVO;
     }
     time_start_mdb = millis();
  }
}

/*********************************************************************************************************
** Nome da Função:      poller_mdb
** Descrição:           Tarefa que manda o poll dos equipamentos ativos a cada 99ms.
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::poller_mdb()
{
   tempo_atual_poll = millis();
   if((tempo_atual_poll-time_start_poll) > 80)
   {
     switch(mdb_task_ctl)
     {
        case 0:
               if( boot_bill == 1 )
               { 
                  // Envia o poll
                  mdb_envia(0x133);
                  mdb_envia(0x033);                    
               }
               mdb_task_ctl++;
               time_start_poll = millis();
               break;
        case 1:
               if( boot_coin == 1 )
               { 
                  // Envia o poll
                  mdb_envia(0x10B);
                  mdb_envia(0x00B);                   
               }
               mdb_task_ctl++;
               time_start_poll = millis();
               break;   
        case 2:
               if( boot_cash == 1 )
               { 
                  // Envia o poll
                  mdb_envia(0x112);
                  mdb_envia(0x012);                   
               }
               mdb_task_ctl = 0;
               time_start_poll = millis();
               break;        
      }
   }
}

/*********************************************************************************************************
** Nome da Função:      verifica_equipamentos_mdb_ativos
** Descrição:           Tarefa que verifica os componentes MDB que estao ligados e os ativa.
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::verifica_equipamentos_mdb_ativos()
{  
  int mensagem;
  
  tempo_atual_verifica_mdb = millis();
  if((tempo_atual_verifica_mdb-time_start_verifica_mdb) > 50)
  {
    time_start_verifica_mdb = millis();
    switch(estado_verifica)
    {
      // Verifica primeiramente o equipamento aceitador de moedas(COIN).
        case 1:
              // Enviando reset para verificar se o componente esta ativo.
              mdb_envia(0x108);
              mdb_envia(0x008);          
              estado_verifica++;
              break;
       // Após enviar o reset, verificar o retorno.  
       case 2:
              // Se receber o ACK indentifica o equipamento como ativo.
              if (mdb_recebe() == 0x100) 
              {
                coin_atv = 1;
              } 
              estado_verifica++;
              break;
       // Verifica primeiramente o equipamento aceitador de notas(BILL).
       case 3:
              // Enviando reset para verificar se o componente esta ativo.
              mdb_envia(0x130);
              mdb_envia(0x030);         
              estado_verifica++;
              break;
       // Após enviar o reset, verificar o retorno.  
       case 4:
              // Se receber o ACK indentifica o equipamento como ativo.
              if (mdb_recebe() == 0x100) 
              {
                bill_atv = 1;
              }else
                  {
                    desabilita_bill();
                  } 
              estado_verifica++;
              break; 
       // Verifica primeiramente o equipamento aceitador de notas(BILL).
       case 5:
              // Enviando reset para verificar se o componente esta ativo.
              mdb_envia(0x110);
              mdb_envia(0x010);        
              estado_verifica++;
              break;
       // Após enviar o reset, verificar o retorno.  
       case 6:
              // Se receber o ACK indentifica o equipamento como ativo.
              if (mdb_recebe() == 0x100) 
              {
                cash_atv = 1;
              } 
              if( coin_atv == INATIVO && cash_atv == INATIVO && bill_atv == INATIVO )
              {
                mdb_sem_equipamentos = ATIVO;
                #ifdef DEBUG_ACEITACAO
                Serial.println(F("MDB SEM EQUIPAMENTOS."));  
                #endif
              }else
                mdb_sem_equipamentos = INATIVO;
                
              estado_verifica = 0;
              time_start_task_mdb = millis();
              break;        
    }
  }
}

/*********************************************************************************************************
** Nome da Função:      task_MDB
** Descrição:           Tarefa que verifica os componentes MDB ativos, faz o bot necessario e faz a 
*                       comunicacao permanente do poll.
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::task()
{   
  verifica_inatividade();
  if( estado_verifica == 0 )
  {
   tempo_atual_task_mdb = millis();
   if((tempo_atual_task_mdb-time_start_task_mdb) > 10)
   { 
     if( boot_mdb == INATIVO )
     { 
        //Verifica se coin ativo.
        if( bill_atv == ATIVO )
        {
          //Se coin ativo.
          if( coin_atv == ATIVO )
          {
            //Aguarda boot do coin.
            if( boot_coin == ATIVO)
            {
              //Inicializa o bill.
              statemachine_bill(); 
            }
          }else if ( cash_atv == ATIVO )
              {
                if( boot_cash == ATIVO )
                {
                  //Inicializa o bill.
                  statemachine_bill();
                }
              }else
                //Inicializa o bill.
                statemachine_bill();        
        }
        //Verifica se coin ativo.
        if( coin_atv == ATIVO )  
        {
           if( cash_atv == ATIVO)
           {
             if( boot_cash == ATIVO )
             {      
                //Entao inicia o coin.
                statemachine_coin();
             }
           }else            
             //Entao inicia o coin.
             statemachine_coin();
        }   
        //Se cashless ativo.
        if( cash_atv == ATIVO )
        {
           //Inicia o cashless. 
           statemachine_cash();
        }     
              
     }else{
        poller_mdb();
        mdb_leitura();        
     }
   }
  }else
      //Tarefa que verifica os componentes MDB que estao ligados e os ativa.
      verifica_equipamentos_mdb_ativos();
       
   pay_out();        
}

/*********************************************************************************************************
** Nome da Função:      valor_tubos
** Descrição:           Método que retorna o valor contido nos tubos.
** Parametro:           Não
** Valor de retorno:    Valor nos tubos.
*********************************************************************************************************/
int MDB::valor_tubos()
{
  int valor_total_tubos = 0;
  
  // Verifica se o coin esta ativo.
  if( coin_esta_ativo() )
  {
    // Verifica os 16 possiveis valores nos tubos.
    for(int i=0;i<16;i++)
    {
      // Serial de debug.
      #ifdef DEBUG_MDB_DETAIL
      Serial.print(F("Status Tubo["));
      Serial.print(i);
      Serial.print(F("]"));
      Serial.println(Status_tubo.Status[i]);
      #endif
      
      // Se o valor no tubo for maior que 0.
      if(Status_tubo.Status[i]>0)
      {
        // Adiciona ao valor total tubos o valor referente a quantidade de moedas no tubo em centavos.
        valor_total_tubos = valor_total_tubos + ((Status_tubo.Status[i]*info_coin.tipo_credito[i])*info_coin.fator_escala);
        //Serial de debug.
        #ifdef DEBUG_MDB_DETAIL
        Serial.print(F("Status Tubo: "));
        Serial.println(Status_tubo.Status[i]);
        Serial.print(F("Total Parcial: "));
        Serial.println(valor_total_tubos);
        #endif
      }         
    }
    
    return valor_total_tubos;   
  // Se coin inativo.  
  }else
    // Retorna 0.
    return 0;
}

/*********************************************************************************************************
** Nome da Função:      recebe_escrow
** Descrição:           Funcao que retira a nota da posicao de escrow e coloca dentro do stacker.
** Parametro:           Não
** Valor de retorno:    Valor nos tubos.
*********************************************************************************************************/
void MDB::recebe_escrow()
{
   mdb_envia(0x135);
   mdb_envia(0x001);
   mdb_envia(0x036);
   delay(15);
   if( mdb_recebe() == 0x100)
   {
     nota_em_custodia = 1;
   }
}

/*********************************************************************************************************
** Nome da Função:      recebe_escrow
** Descrição:           Funcao que retira a nota da posicao de escrow e coloca dentro do stacker.
** Parametro:           Não
** Valor de retorno:    Não
*********************************************************************************************************/
void MDB::rejeita_escrow()
{
   mdb_envia(0x135);
   mdb_envia(0x000);
   mdb_envia(0x035);
   delay(15);
   if( mdb_recebe() == 0x100)
   {
     nota_em_custodia = 0;
   }
}

