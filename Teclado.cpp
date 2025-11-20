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

// Pinos do teclado matricial
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
** Descrição:            Construtor da classe.
** Parametro:            Não.
** Valor de retorno:     Não.
*********************************************************************************************************/
Teclado::Teclado()
{
  customKeypad.begin(makeKeymap(hexaKeys));
}

/*********************************************************************************************************
** Nome da Função:       status_teclado
** Descrição:            Status do teclado.
** Parametro:            Não.
** Valor de retorno:     Estado do keypad.
*********************************************************************************************************/
int Teclado::status_teclado()
{
  return customKeypad.getState(); 
}

/*********************************************************************************************************
** Nome da Função:       leitura
** Descrição:            Leitura do teclado com debounce.
** Parametro:            Não.
** Valor de retorno:     Tecla pressionada ou NO_KEY.
*********************************************************************************************************/
char Teclado::leitura()
{
  char customKey = customKeypad.getKey();

  if ( customKey )
  {
    tecla_selecionada = customKey;
    tempo_inicio_teclado = millis();
  }

  if ( customKeypad.getState() == 3 ) 
  {
    pulso = (tempo_atual_teclado-tempo_inicio_teclado);
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
** Descrição:            Leitura do teclado com hold.
** Parametro:            Não.
** Valor de retorno:     Tecla mantida ou NO_KEY.
*********************************************************************************************************/
char Teclado::leitura_hold()
{
  char customKey = customKeypad.getKey();

  if ( customKey )
  {
    tecla_selecionada = customKey;
    tempo_inicio_teclado = millis();
  }

  if ( customKeypad.getState() == 2 || customKeypad.getState() == 1 ) 
  {
    return tecla_selecionada;
  }else return NO_KEY;
}


/*********************************************************************************************************
** Nome da Função:       valor_lido
** Descrição:            Retorna tempo de pulso e reseta variável.
** Parametro:            Não.                         
** Valor de retorno:     Pulso em ms.
*********************************************************************************************************/
int Teclado::valor_lido()
{ 
  tecla_selecionada = NO_KEY;
  return pulso;
}

/*********************************************************************************************************
** Nome da Função:       selecao_de_caracteres
** Descrição:            Seleção de caracteres.
** Parametro:            Não.
** Valor de retorno:     NO_KEY.
*********************************************************************************************************/
char Teclado::selecao_de_caracteres()
{
  return NO_KEY;
}
