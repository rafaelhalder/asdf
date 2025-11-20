/****************************************Copyright (c)*************************************************
**                      Power Vending - A. R. M. Venda Automatica Ltda.
**                             http://www.powervending.com.br
**--------------Informação do Arquivo -----------------------------------------------------------------
** Nome do Arquivo:          ErrorLog.h
** Data Ultima Modificação:  20-11-24
** Ultima Versão:            Sim
** Descrição:                Sistema log erros EEPROM
**                           Registra últimos erros para debugging
**------------------------------------------------------------------------------------------------------
** Criado por:          Rafael Henrique (rafaelhalder@gmail.com)
** Data de Criação:     20-11-24
********************************************************************************************************/

#ifndef ErrorLog_h
#define ErrorLog_h

#include <Arduino.h>
#include <EEPROM.h>

// Sistema log de erros
// Registra erros na EEPROM para análise posterior

// Endereços EEPROM log (região 1500-1599)
#define EEPROM_ERROR_LOG_START 1500
#define EEPROM_ERROR_LOG_COUNT 1599
#define MAX_ERROR_LOGS 10

// Códigos erro
enum ErrorCode {
  ERR_NONE = 0,
  ERR_MDB_TIMEOUT = 1,           // MDB não respondeu
  ERR_MDB_BILL_TIMEOUT = 2,      // Bill não respondeu
  ERR_MDB_COIN_TIMEOUT = 3,      // Coin não respondeu
  ERR_MDB_CASH_TIMEOUT = 4,      // Cashless não respondeu
  ERR_MDB_CHK_FAIL = 5,          // Checksum MDB inválido
  ERR_SENSOR_TIMEOUT = 6,        // Sensor não detectou produto
  ERR_MOTOR_TIMEOUT = 7,         // Motor não completou
  ERR_EEPROM_WRITE_FAIL = 8,     // Falha ao gravar EEPROM
  ERR_MEMORY_LOW = 9,            // Memória RAM baixa
  ERR_GENERIC = 99               // Erro genérico
};

// Estrutura de um registro de erro
struct ErrorEntry {
  byte error_code;          // Código do erro (ErrorCode)
  unsigned long timestamp;  // Timestamp em millis()
  byte extra_info;          // Informação adicional (opcional)
};

class ErrorLog {
  private:
    // Calcula endereço EEPROM para um índice de erro
    int getErrorAddress(int index) {
      return EEPROM_ERROR_LOG_START + (index * sizeof(ErrorEntry));
    }
    
  public:
    // Construtor
    ErrorLog() {}
    
    // Registra um novo erro na EEPROM
    // Parâmetros:
    //   code: código do erro (ErrorCode)
    //   extra: informação adicional opcional (default 0)
    void logError(ErrorCode code, byte extra = 0) {
      // Lê contador de erros
      int error_count = EEPROM.read(EEPROM_ERROR_LOG_COUNT);
      
      // Calcula índice circular (últimos 10)
      int index = error_count % MAX_ERROR_LOGS;
      
      // Cria entrada
      ErrorEntry entry;
      entry.error_code = (byte)code;
      entry.timestamp = millis();
      entry.extra_info = extra;
      
      // Grava na EEPROM
      int addr = getErrorAddress(index);
      EEPROM.put(addr, entry);
      
      // Atualiza contador
      error_count++;
      if (error_count > 255) error_count = 0; // Wrap around
      EEPROM.write(EEPROM_ERROR_LOG_COUNT, error_count);
      
      // Debug serial
      #ifdef DEBUG_MDB
      Serial.print(F("ERROR LOGGED: Code="));
      Serial.print(code);
      Serial.print(F(" Extra="));
      Serial.print(extra);
      Serial.print(F(" Time="));
      Serial.println(entry.timestamp);
      #endif
    }
    
    // Retorna o número total de erros registrados
    int getErrorCount() {
      return EEPROM.read(EEPROM_ERROR_LOG_COUNT);
    }
    
    // Lê um erro específico do log
    // Parâmetros:
    //   index: índice do erro (0 = mais antigo nos últimos 10)
    // Retorna:
    //   ErrorEntry com os dados do erro
    ErrorEntry getError(int index) {
      ErrorEntry entry;
      if (index < 0 || index >= MAX_ERROR_LOGS) {
        entry.error_code = ERR_NONE;
        entry.timestamp = 0;
        entry.extra_info = 0;
        return entry;
      }
      
      int addr = getErrorAddress(index);
      EEPROM.get(addr, entry);
      return entry;
    }
    
    // Imprime todos os erros via Serial
    void printErrors() {
      Serial.println(F("===== ERROR LOG ====="));
      int total = getErrorCount();
      Serial.print(F("Total erros: "));
      Serial.println(total);
      
      int start_idx = (total > MAX_ERROR_LOGS) ? (total % MAX_ERROR_LOGS) : 0;
      int count = min(total, MAX_ERROR_LOGS);
      
      for (int i = 0; i < count; i++) {
        int idx = (start_idx + i) % MAX_ERROR_LOGS;
        ErrorEntry entry = getError(idx);
        
        Serial.print(F("["));
        Serial.print(i);
        Serial.print(F("] Code: "));
        Serial.print(entry.error_code);
        Serial.print(F(" ("));
        printErrorName(entry.error_code);
        Serial.print(F(") Time: "));
        Serial.print(entry.timestamp);
        Serial.print(F(" Extra: "));
        Serial.println(entry.extra_info);
      }
      Serial.println(F("====================="));
    }
    
    // Limpa o log de erros
    void clearLog() {
      EEPROM.write(EEPROM_ERROR_LOG_COUNT, 0);
      
      // Opcional: zerar entradas (não necessário, mas limpa)
      for (int i = 0; i < MAX_ERROR_LOGS; i++) {
        ErrorEntry empty;
        empty.error_code = ERR_NONE;
        empty.timestamp = 0;
        empty.extra_info = 0;
        int addr = getErrorAddress(i);
        EEPROM.put(addr, empty);
      }
      
      Serial.println(F("Error log cleared"));
    }
    
    // Retorna o nome do erro
    void printErrorName(byte code) {
      switch(code) {
        case ERR_NONE:              Serial.print(F("No error")); break;
        case ERR_MDB_TIMEOUT:       Serial.print(F("MDB timeout")); break;
        case ERR_MDB_BILL_TIMEOUT:  Serial.print(F("Bill timeout")); break;
        case ERR_MDB_COIN_TIMEOUT:  Serial.print(F("Coin timeout")); break;
        case ERR_MDB_CASH_TIMEOUT:  Serial.print(F("Cash timeout")); break;
        case ERR_MDB_CHK_FAIL:      Serial.print(F("CHK fail")); break;
        case ERR_SENSOR_TIMEOUT:    Serial.print(F("Sensor timeout")); break;
        case ERR_MOTOR_TIMEOUT:     Serial.print(F("Motor timeout")); break;
        case ERR_EEPROM_WRITE_FAIL: Serial.print(F("EEPROM fail")); break;
        case ERR_MEMORY_LOW:        Serial.print(F("Memory low")); break;
        case ERR_GENERIC:           Serial.print(F("Generic")); break;
        default:                    Serial.print(F("Unknown")); break;
      }
    }
};

// ============================================================================
// EXEMPLOS DE USO
// ============================================================================

/*
// Instanciar no início do programa
ErrorLog errorLog;

// Exemplo 1: Registrar erro MDB timeout
if (mdb_timeout) {
  errorLog.logError(ERR_MDB_TIMEOUT);
  // Tentar recuperar ao invés de apenas resetar
  mdb.reset();
}

// Exemplo 2: Registrar erro com informação extra
if (sensor_timeout) {
  errorLog.logError(ERR_SENSOR_TIMEOUT, produto_selecionado);
  // produto_selecionado fica armazenado como extra_info
}

// Exemplo 3: Ver erros via Serial
if (Serial.available() && Serial.read() == 'E') {
  errorLog.printErrors();
}

// Exemplo 4: Limpar log
if (Serial.available() && Serial.read() == 'C') {
  errorLog.clearLog();
}

// Exemplo 5: No lugar de reset imediato
void verifica_inatividade() {
  if (sem_retorno_mdb == ATIVO) {
    // ANTES: delay(5000); wdt_enable(WDTO_15MS); while(1) {}
    
    // AGORA: Registra erro antes de resetar
    errorLog.logError(ERR_MDB_TIMEOUT);
    Serial.println(F("MDB RESET - Aguardando 5s..."));
    unsigned long inicio = millis();
    while(millis() - inicio < 5000) {
      if (Serial1.available()) Serial1.read();
    }
    wdt_enable(WDTO_15MS);
    while(1) {}
  }
}
*/

#endif
