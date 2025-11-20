/****************************************Copyright (c)*************************************************
**                      Power Vending - A. R. M. Venda Automatica Ltda.
**                             http://www.powervending.com.br
**--------------Informação do Arquivo -----------------------------------------------------------------
** Nome do Arquivo:          Teclado.h
** Data Ultima Modificação:  24-07-18
** Ultima Versão:            Sim
** Descrição:                Biblioteca de controle do teclado.          
**------------------------------------------------------------------------------------------------------
** Criado por:          João Henrique Moreira Gabardo <jhmfg2014@outlook.com>
** Data de Criação:     27-07-18      
********************************************************************************************************/

#ifndef Teclado_h
#define Teclado_h

#include <Arduino.h>

class Teclado
{
  public:
      // Construtor da classe
      Teclado();

            char leitura();
      char leitura_hold();
      int valor_lido();
      char selecao_de_caracteres();
      int status_teclado();
      
  private:
      char tecla_selecionada;
      long tempo_inicio_teclado, tempo_atual_teclado = millis();
      int pulso = 0;
  
};

#endif
