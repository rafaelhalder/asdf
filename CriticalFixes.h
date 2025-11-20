/****************************************Copyright (c)*************************************************
**                      Power Vending - A. R. M. Venda Automatica Ltda.
**                             http://www.powervending.com.br
**--------------Informação do Arquivo -----------------------------------------------------------------
** Nome do Arquivo:          CriticalFixes.h
** Data Ultima Modificação:  20-11-24
** Ultima Versão:            Sim
** Descrição:                Implementação das correções críticas identificadas nas simulações
**                           - Timeout do sensor (Cenário 2)
**                           - Persistência de transação (Cenário 3)
**                           - Proteção contra race conditions (Cenário 1)
**------------------------------------------------------------------------------------------------------
** Criado por:          GitHub Copilot
** Data de Criação:     20-11-24
********************************************************************************************************/

#ifndef CriticalFixes_h
#define CriticalFixes_h

#include <Arduino.h>
#include <EEPROM.h>

// ============================================================================
// CORREÇÃO CRÍTICA 1: PERSISTÊNCIA DE TRANSAÇÃO
// Evita perda de dinheiro se sistema resetar durante venda
// ============================================================================

// Endereço EEPROM para transação em andamento (1620-1629)
#define EEPROM_TRANSACTION_ADDR 1620

// Estrutura de transação em andamento
struct TransacaoEmAndamento {
  bool ativa;                   // Transação em progresso?
  int valor_inserido;           // Valor que o usuário colocou
  int produto_selecionado;      // Qual produto foi escolhido
  unsigned long timestamp;      // Quando começou (millis)
  byte checksum;                // Validação de integridade
  
  // Construtor
  TransacaoEmAndamento() : 
    ativa(false),
    valor_inserido(0),
    produto_selecionado(0),
    timestamp(0),
    checksum(0)
  {}
  
  // Calcula checksum para validação
  byte calcular_checksum() {
    return (byte)((ativa ? 1 : 0) + 
                  (valor_inserido & 0xFF) + 
                  (produto_selecionado & 0xFF) + 
                  (timestamp & 0xFF));
  }
  
  // Valida integridade
  bool validar() {
    return checksum == calcular_checksum();
  }
};

// Classe para gerenciar transações
class TransactionManager {
  private:
    TransacaoEmAndamento transacao_atual;
    
  public:
    // Construtor
    TransactionManager() {}
    
    // Inicia uma nova transação
    void iniciar(int valor, int produto) {
      transacao_atual.ativa = true;
      transacao_atual.valor_inserido = valor;
      transacao_atual.produto_selecionado = produto;
      transacao_atual.timestamp = millis();
      transacao_atual.checksum = transacao_atual.calcular_checksum();
      
      // Salva na EEPROM
      EEPROM.put(EEPROM_TRANSACTION_ADDR, transacao_atual);
      
      #ifdef DEBUG_MDB
      Serial.print(F("Transaction started: R$"));
      Serial.print(valor / 100);
      Serial.print(F("."));
      Serial.print(valor % 100);
      Serial.print(F(" Product: "));
      Serial.println(produto);
      #endif
    }
    
    // Finaliza transação (sucesso)
    void finalizar() {
      transacao_atual.ativa = false;
      transacao_atual.checksum = transacao_atual.calcular_checksum();
      EEPROM.put(EEPROM_TRANSACTION_ADDR, transacao_atual);
      
      #ifdef DEBUG_MDB
      Serial.println(F("Transaction completed successfully"));
      #endif
    }
    
    // Verifica se há transação incompleta após reset
    // Retorna valor a ser retornado ao usuário (0 se não há transação)
    int recuperar_transacao_incompleta() {
      TransacaoEmAndamento t;
      EEPROM.get(EEPROM_TRANSACTION_ADDR, t);
      
      // Valida integridade
      if (!t.validar()) {
        #ifdef DEBUG_MDB
        Serial.println(F("No valid incomplete transaction found"));
        #endif
        return 0;
      }
      
      // Verifica se está ativa
      if (t.ativa) {
        Serial.println(F("*** INCOMPLETE TRANSACTION DETECTED ***"));
        Serial.print(F("Value: R$"));
        Serial.print(t.valor_inserido / 100);
        Serial.print(F("."));
        Serial.println(t.valor_inserido % 100);
        Serial.print(F("Product: "));
        Serial.println(t.produto_selecionado);
        Serial.print(F("Time: "));
        Serial.println(t.timestamp);
        
        // Limpa transação
        t.ativa = false;
        t.checksum = t.calcular_checksum();
        EEPROM.put(EEPROM_TRANSACTION_ADDR, t);
        
        // Retorna valor para reembolso
        return t.valor_inserido;
      }
      
      return 0;
    }
    
    // Verifica se há transação ativa
    bool tem_transacao_ativa() {
      return transacao_atual.ativa;
    }
};

// ============================================================================
// CORREÇÃO CRÍTICA 2: TIMEOUT DO SENSOR
// Evita sistema travar se produto não cair
// ============================================================================

// Configurações de timeout
#define SENSOR_TIMEOUT_MS 10000       // 10 segundos para produto cair
#define MOTOR_TIMEOUT_MS 5000         // 5 segundos para motor girar

// Classe para gerenciar timeout do sensor
class SensorTimeout {
  private:
    unsigned long start_time;
    bool monitoring;
    
  public:
    // Construtor
    SensorTimeout() : start_time(0), monitoring(false) {}
    
    // Inicia monitoramento
    void start() {
      start_time = millis();
      monitoring = true;
      
      #ifdef DEBUG_MDB
      Serial.println(F("Sensor timeout monitoring started"));
      #endif
    }
    
    // Para monitoramento
    void stop() {
      monitoring = false;
      
      #ifdef DEBUG_MDB
      Serial.println(F("Sensor timeout monitoring stopped"));
      #endif
    }
    
    // Verifica se houve timeout
    bool timeout_occurred() {
      if (!monitoring) return false;
      
      return (millis() - start_time) > SENSOR_TIMEOUT_MS;
    }
    
    // Retorna tempo decorrido
    unsigned long elapsed() {
      if (!monitoring) return 0;
      return millis() - start_time;
    }
    
    // Verifica se está monitorando
    bool is_monitoring() {
      return monitoring;
    }
};

// ============================================================================
// CORREÇÃO CRÍTICA 3: PROTEÇÃO CONTRA RACE CONDITIONS
// Evita problemas com múltiplas moedas rápidas
// ============================================================================

// Classe para gerenciar inserção de valores com segurança
class SafeValueManager {
  private:
    volatile int valor_protegido;
    
  public:
    // Construtor
    SafeValueManager() : valor_protegido(0) {}
    
    // Adiciona valor com proteção
    void adicionar(int valor) {
      // Desabilita interrupts durante operação crítica
      noInterrupts();
      valor_protegido += valor;
      interrupts();
      
      #ifdef DEBUG_MDB
      Serial.print(F("Value added: "));
      Serial.print(valor / 100);
      Serial.print(F("."));
      Serial.print(valor % 100);
      Serial.print(F(" Total: "));
      Serial.print(valor_protegido / 100);
      Serial.print(F("."));
      Serial.println(valor_protegido % 100);
      #endif
    }
    
    // Lê valor com proteção
    int ler() {
      int temp;
      noInterrupts();
      temp = valor_protegido;
      interrupts();
      return temp;
    }
    
    // Define valor com proteção
    void definir(int valor) {
      noInterrupts();
      valor_protegido = valor;
      interrupts();
    }
    
    // Zera valor
    void zerar() {
      noInterrupts();
      valor_protegido = 0;
      interrupts();
    }
};

// ============================================================================
// FUNÇÕES AUXILIARES
// ============================================================================

// Verifica se é seguro resetar o sistema
bool pode_resetar_sistema(bool em_venda, int controle_vmc) {
  // Não resetar se:
  // - Está em venda (em_venda == true)
  // - Estado da máquina indica operação crítica (controle_vmc != 0 && controle_vmc != 20)
  
  if (em_venda) {
    Serial.println(F("RESET BLOCKED: Sale in progress"));
    return false;
  }
  
  if (controle_vmc != 0 && controle_vmc != 20) {
    Serial.println(F("RESET BLOCKED: Critical state"));
    return false;
  }
  
  return true;
}

#endif
