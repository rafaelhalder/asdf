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

      // Função que le o valor digitado no teclado.
      char leitura();
      
      // Função que le o valor digitado no teclado.
      char leitura_hold();

      // Função que reseta o valor da variavel após a leitura.
      int valor_lido();

      // Função onde seleciona um caracter.
      char selecao_de_caracteres();

      // Retorna o status do hold.
      int status_teclado();
      
  private:
      // Armazena a tecla selecionada.
      char tecla_selecionada;
      // Temporizadores.
      long tempo_inicio_teclado, tempo_atual_teclado = millis();
      // Pulso.
      int pulso = 0;
  
};

#endif
