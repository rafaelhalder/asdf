/****************************************Copyright (c)*************************************************
**                      Power Vending - A. R. M. Venda Automatica Ltda.
**                             http://www.powervending.com.br
**--------------Informação do Arquivo -----------------------------------------------------------------
** Nome do Arquivo:          Teclado.cpp
** Data Ultima Modificação:  24-07-18
** Ultima Versão:            Sim
** Descrição:                Biblioteca de controle do teclado(Header).          
**------------------------------------------------------------------------------------------------------
** Criado por:          João Henrique Moreira Gabardo <jhmfg2014@outlook.com>
** Data de Criação:     27-07-18      
********************************************************************************************************/

#include "Arduino.h"
#include "Teclado.h"
#include "Keypad.h"

#define ACRESCENTA 1
#define DECREMENTA 0
#define INATIVO 0
#define OCIOSO -1
#define PRE_HOLD 1
#define IN_HOLD 2

//Pinos teclado.
//Pinos teclado.
/*#define C1 49
#define C2 47
#define C3 45
#define C4 43
#define R1 41
#define R2 39
#define R3 37*/
#define R1 37
#define R2 39
#define R3 41
#define R4 43
#define C1 45
#define C2 47
#define C3 49

// Linhas keypad biblioteca.
#define ROWS  4  
// Colunas keypad biblioteca.
#define COLS  3  
     
//Valores para teclas
char hexaKeys[ROWS][COLS] = {                         
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'A','0','B'}
};
// Pinos das linhas do teclado matricial
byte rowPins[ROWS] = {R1, R2, R3, R4};          
// Pinos das colunas do teclado matricial
byte colPins[COLS] = {C1, C2, C3}; 

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

/*********************************************************************************************************
** Nome da Função:       Teclado
** Descrição:            Função que é o construtor da classe.
** Parametro:            Não.
** Valor de retorno:     Não.
*********************************************************************************************************/
Teclado::Teclado()
{
  customKeypad.begin(makeKeymap(hexaKeys));
}

/*********************************************************************************************************
** Nome da Função:       status_teclado
** Descrição:            Função que le o valor digitado no teclado.
** Parametro:            Não.
** Valor de retorno:     retorna o valor recebido do teclado(keypad).
*********************************************************************************************************/
int Teclado::status_teclado()
{
  return customKeypad.getState(); 
}

/*********************************************************************************************************
** Nome da Função:       leitura
** Descrição:            Função que le o valor digitado no teclado.
** Parametro:            Não.
** Valor de retorno:     retorna o valor recebido do teclado(keypad).
*********************************************************************************************************/
char Teclado::leitura()
{
  // Retorna o valor recebido pelo teclado.
  char customKey = customKeypad.getKey();

  // Se tecla pressionada.
  if ( customKey )
  {
    // Grava qual tecla foi pressionada.
    tecla_selecionada = customKey;
    // Inicia temporizador
    tempo_inicio_teclado = millis();
  }

  if ( customKeypad.getState() == 3 ) 
  {
    pulso = (tempo_atual_teclado-tempo_inicio_teclado);
    //Serial.println("PULSO: " + (String)(tempo_atual_teclado-tempo_inicio_teclado));
  }

  tempo_atual_teclado = millis();
  if ( tempo_atual_teclado-tempo_inicio_teclado > 60 )
  {
    if ( customKeypad.getState() == 3 ) 
    {
      return tecla_selecionada;
    }else return NO_KEY;
  }

  return NO_KEY;
}

/*********************************************************************************************************
** Nome da Função:       leitura_hold
** Descrição:            Função que le o valor digitado no teclado.
** Parametro:            Não.
** Valor de retorno:     retorna o valor recebido do teclado(keypad).
*********************************************************************************************************/
char Teclado::leitura_hold()
{
  // Retorna o valor recebido pelo teclado.
  char customKey = customKeypad.getKey();

  // Se tecla pressionada.
  if ( customKey )
  {
    // Grava qual tecla foi pressionada.
    tecla_selecionada = customKey;
    // Inicia temporizador
    tempo_inicio_teclado = millis();
  }
  // Verifica se o botão esta sendo pressionado.
  if ( customKeypad.getState() == 2 || customKeypad.getState() == 1 ) 
  {
    return tecla_selecionada;
  }else return NO_KEY;
}


/*************************************************************************************************************************************************************************
** Nome da Função:       valor_lido
** Descrição:            Função que reseta o valor da variavel após a leitura.
** Parametro:            Não.                         
** Valor de retorno:     Não
***************************************************************************************************************************************************************************/
int Teclado::valor_lido()
{ 
  //Serial.println("PULSO: " + (String)pulso + " ms");
  tecla_selecionada = NO_KEY;
}

/*********************************************************************************************************
** Nome da Função:       
** Descrição:            
** Parametro:            Não.
** Valor de retorno:     Não.
*********************************************************************************************************/
char Teclado::selecao_de_caracteres()
{
  
}
