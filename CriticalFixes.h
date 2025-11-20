/****************************************Copyright (c)*************************************************
**                      Power Vending - A. R. M. Venda Automatica Ltda.
**                             http://www.powervending.com.br
**--------------Informação do Arquivo -----------------------------------------------------------------
** Nome do Arquivo:          CriticalFixes.h
** Data Ultima Modificação:  20-11-24
** Ultima Versão:            Sim
** Descrição:                Correções críticas identificadas nas simulações
**                           - Timeout do sensor
**                           - Persistência de transação
**                           - Proteção race conditions
**------------------------------------------------------------------------------------------------------
** Criado por:          Rafael Henrique (rafaelhalder@gmail.com)
** Data de Criação:     20-11-24
********************************************************************************************************/

#ifndef CriticalFixes_h
#define CriticalFixes_h

#include <Arduino.h>
#include <EEPROM.h>

// Persistência de transação
// Evita perda de dinheiro se sistema resetar durante venda

// Endereço EEPROM transação em andamento (1620-1629)
#define EEPROM_TRANSACTION_ADDR 1620

// Estrutura transação em andamento
struct TransacaoEmAndamento {
  bool ativa;                   // Transação em progresso
  int valor_inserido;           // Valor inserido
  int produto_selecionado;      // Produto escolhido
  unsigned long timestamp;      // Timestamp início
  byte checksum;                // Validação integridade
  
  // Construtor
  TransacaoEmAndamento() : 
    ativa(false),
    valor_inserido(0),
    produto_selecionado(0),
    timestamp(0),
    checksum(0)
  {}
  
  // Calcula checksum
  byte calcular_checksum() {
    return (byte)((ativa ? 1 : 0) + 
                  (valor_inserido & 0xFF) + 
                  (produto_selecionado & 0xFF) + 
                  (timestamp & 0xFF));
  }
  
  // Valida checksum
  bool validar() {
    return checksum == calcular_checksum();
  }
};

// Gerenciador de transações
class TransactionManager {
  private:
    TransacaoEmAndamento transacao_atual;
    
  public:
    TransactionManager() {}
    
    // Inicia transação
    void iniciar(int valor, int produto) {
      transacao_atual.ativa = true;
      transacao_atual.valor_inserido = valor;
      transacao_atual.produto_selecionado = produto;
      transacao_atual.timestamp = millis();
      transacao_atual.checksum = transacao_atual.calcular_checksum();
      
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
    
    // Finaliza transação
    void finalizar() {
      transacao_atual.ativa = false;
      transacao_atual.checksum = transacao_atual.calcular_checksum();
      EEPROM.put(EEPROM_TRANSACTION_ADDR, transacao_atual);
      
      #ifdef DEBUG_MDB
      Serial.println(F("Transaction completed successfully"));
      #endif
    }
    
    // Recupera transação incompleta após reset
    // Retorna valor (0 se não há transação)
    int recuperar_transacao_incompleta() {
      TransacaoEmAndamento t;
      EEPROM.get(EEPROM_TRANSACTION_ADDR, t);
      
      if (!t.validar()) {
        #ifdef DEBUG_MDB
        Serial.println(F("No valid incomplete transaction found"));
        #endif
        return 0;
      }
      
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
        
        t.ativa = false;
        t.checksum = t.calcular_checksum();
        EEPROM.put(EEPROM_TRANSACTION_ADDR, t);
        
        return t.valor_inserido;
      }
      
      return 0;
    }
    
    // Verifica transação ativa
    bool tem_transacao_ativa() {
      return transacao_atual.ativa;
    }
};

// Timeout do sensor
// Evita sistema travar se produto não cair

#define SENSOR_TIMEOUT_MS 10000       // 10s produto cair
#define MOTOR_TIMEOUT_MS 5000         // 5s motor girar

// Gerenciador timeout sensor
class SensorTimeout {
  private:
    unsigned long start_time;
    bool monitoring;
    
  public:
    SensorTimeout() : start_time(0), monitoring(false) {}
    
    void start() {
      start_time = millis();
      monitoring = true;
      
      #ifdef DEBUG_MDB
      Serial.println(F("Sensor timeout monitoring started"));
      #endif
    }
    
    void stop() {
      monitoring = false;
      
      #ifdef DEBUG_MDB
      Serial.println(F("Sensor timeout monitoring stopped"));
      #endif
    }
    
    bool timeout_occurred() {
      if (!monitoring) return false;
      
      return (millis() - start_time) > SENSOR_TIMEOUT_MS;
    }
    
    unsigned long elapsed() {
      if (!monitoring) return 0;
      return millis() - start_time;
    }
    
    bool is_monitoring() {
      return monitoring;
    }
};

// Proteção race conditions
// Evita problemas com múltiplas moedas rápidas

// Gerenciador seguro de valores
class SafeValueManager {
  private:
    volatile int valor_protegido;
    
  public:
    SafeValueManager() : valor_protegido(0) {}
    
    void adicionar(int valor) {
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
    
    int ler() {
      int temp;
      noInterrupts();
      temp = valor_protegido;
      interrupts();
      return temp;
    }
    
    void definir(int valor) {
      noInterrupts();
      valor_protegido = valor;
      interrupts();
    }
    
    void zerar() {
      noInterrupts();
      valor_protegido = 0;
      interrupts();
    }
};

// Verifica se pode resetar sistema
bool pode_resetar_sistema(bool em_venda, int controle_vmc) {
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
