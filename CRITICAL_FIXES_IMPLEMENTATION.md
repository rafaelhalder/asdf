# Guia de Implementação - Correções Críticas

## 📋 Objetivo

Este documento fornece **instruções passo a passo** para implementar as correções críticas identificadas nas simulações de segurança.

---

## 🔴 Correção 1: Timeout do Sensor (CRÍTICO)

### Problema
Sistema trava indefinidamente se produto não cai. Usuário perde dinheiro.

### Solução Implementada
Adicionar timeout de 10 segundos com retorno automático de dinheiro.

### Código para Adicionar

**Passo 1:** Incluir o header no início do arquivo `.ino`:
```cpp
#include "CriticalFixes.h"
```

**Passo 2:** Declarar variáveis globais (após outras declarações):
```cpp
// Gerenciamento de timeout do sensor
SensorTimeout sensor_timeout;
```

**Passo 3:** Localizar a função `statemachine_vmc()` e encontrar o estado onde aguarda queda do produto.

Exemplo de onde adicionar (procure código similar):
```cpp
// ❌ CÓDIGO ATUAL (SEM TIMEOUT)
case 100:  // ou outro número que aguarda sensor
  tempo_atual_infra = millis();
  if((tempo_atual_infra-time_start_infra) > 200) {
    if (sensor_queda_infra.get_evento_disponivel()) {
      // Produto detectado!
      sensor_queda_infra.finaliza_leitura();
      controle_vmc = 58;  // Próximo estado
      digitalWrite(RELE_2, LOW);
    }
    // ⚠️ FALTA TIMEOUT AQUI!
  }
  break;
```

**Passo 4:** Adicionar código de timeout:
```cpp
// ✅ CÓDIGO CORRIGIDO (COM TIMEOUT)
case 100:  // ou outro número que aguarda sensor
  tempo_atual_infra = millis();
  if((tempo_atual_infra-time_start_infra) > 200) {
    if (sensor_queda_infra.get_evento_disponivel()) {
      // Produto detectado - SUCESSO!
      Serial.println(F("Product detected successfully"));
      sensor_queda_infra.finaliza_leitura();
      sensor_timeout.stop();  // Para monitoramento
      controle_vmc = 58;  // Próximo estado (sucesso)
      digitalWrite(RELE_2, LOW);
    }
    else if (sensor_timeout.timeout_occurred()) {
      // ⚠️ TIMEOUT - Produto não caiu!
      Serial.println(F("*** SENSOR TIMEOUT - PRODUCT DID NOT FALL ***"));
      
      // Para motor
      digitalWrite(RELE_2, LOW);
      
      // Finaliza leitura do sensor
      sensor_queda_infra.finaliza_leitura();
      sensor_timeout.stop();
      
      // Registra erro
      errorLog.logError(ERR_SENSOR_TIMEOUT, produto_selecionado);
      
      // Incrementa contador de falhas
      qtd_eventos_falha++;
      escreve_eeprom(EEPROM_ADDR_QTD_EVENTOS_FALHA_1,
                     EEPROM_ADDR_QTD_EVENTOS_FALHA_2,
                     qtd_eventos_falha);
      
      // Mostra mensagem de erro
      lcd2.clear();
      lcd2.setCursor(0,1);
      lcd2.print(F("  ERRO: PRODUTO"));
      lcd2.setCursor(0,2);
      lcd2.print(F("   NAO LIBERADO"));
      
      // Retorna dinheiro ao usuário
      if (valor_inserido > 0) {
        Serial.print(F("Refunding: R$"));
        Serial.println(valor_inserido / 100);
        mdb.entregar_troco(valor_inserido);
      }
      
      // Zera variáveis
      valor_inserido = 0;
      
      // Volta para IDLE após 3 segundos
      delay(3000);
      controle_vmc = 0;
    }
  }
  break;
```

**Passo 5:** No estado que INICIA a dispensação do produto, adicionar:
```cpp
case 57:  // ou outro número que inicia dispensação
  // ... código existente ...
  
  sensor_queda_infra.realiza_leitura();
  sensor_timeout.start();  // ← ADICIONAR ESTA LINHA
  
  controle_vmc = 100;
  break;
```

---

## 🔴 Correção 2: Reset Durante Venda (CRÍTICO)

### Problema
Se sistema reseta durante venda, usuário perde o dinheiro inserido.

### Solução Implementada
Salvar estado da transação na EEPROM antes de operações críticas.

### Código para Adicionar

**Passo 1:** Incluir o header (se ainda não incluiu):
```cpp
#include "CriticalFixes.h"
```

**Passo 2:** Declarar variável global:
```cpp
// Gerenciamento de transações
TransactionManager transaction_manager;
```

**Passo 3:** No `setup()`, adicionar recuperação de transações:
```cpp
void setup() {
  // ... código de inicialização existente ...
  
  Serial.begin(115200);
  
  // ← ADICIONAR APÓS Serial.begin()
  // Verifica se há transação incompleta após reset
  int valor_recuperado = transaction_manager.recuperar_transacao_incompleta();
  
  if (valor_recuperado > 0) {
    // Transação incompleta detectada!
    Serial.println(F("========================================"));
    Serial.println(F("INCOMPLETE TRANSACTION RECOVERED"));
    Serial.print(F("Refunding: R$"));
    Serial.print(valor_recuperado / 100);
    Serial.print(F("."));
    Serial.println(valor_recuperado % 100);
    Serial.println(F("========================================"));
    
    // Aguarda MDB inicializar
    delay(5000);
    
    // Retorna dinheiro ao usuário
    mdb.entregar_troco(valor_recuperado);
    
    Serial.println(F("Refund completed"));
  }
  
  // ... resto do setup ...
}
```

**Passo 4:** Quando usuário insere dinheiro e seleciona produto, ANTES de ligar motor:
```cpp
// Exemplo: quando vai iniciar venda
// Localizar código que verifica crédito e inicia dispensação

if (valor_inserido >= preco_produto) {
  // ← ADICIONAR ANTES DE LIGAR MOTOR
  // Salva transação antes de operação crítica
  transaction_manager.iniciar(valor_inserido, produto_selecionado);
  
  // Liga motor
  digitalWrite(MOTOR, HIGH);
  
  // ... continua ...
}
```

**Passo 5:** Quando venda é finalizada COM SUCESSO:
```cpp
// Após produto cair e ser detectado
if (sensor_queda_infra.get_evento_disponivel()) {
  // Produto caiu com sucesso
  
  // Atualiza contabilidade
  receita_total += preco_produto;
  estoque--;
  
  // ← ADICIONAR AQUI
  // Finaliza transação (marca como completa)
  transaction_manager.finalizar();
  
  // ... continua ...
}
```

**Passo 6:** Quando venda é CANCELADA ou DÁ ERRO:
```cpp
// Se timeout, erro, ou cancelamento
if (sensor_timeout.timeout_occurred() || cancelamento) {
  // Retorna dinheiro
  mdb.entregar_troco(valor_inserido);
  
  // ← ADICIONAR AQUI
  // Finaliza transação (não foi concluída, mas já tratada)
  transaction_manager.finalizar();
  
  controle_vmc = 0;
}
```

---

## 🟡 Correção 3: Race Conditions (Média Prioridade)

### Problema
Múltiplas moedas inseridas rapidamente podem causar valores incorretos.

### Solução Implementada
Proteger operações críticas com `noInterrupts()`.

### Código para Adicionar

**Passo 1:** Incluir o header:
```cpp
#include "CriticalFixes.h"
```

**Passo 2:** Declarar variável global:
```cpp
// Gerenciamento seguro de valor inserido
SafeValueManager safe_valor_inserido;
```

**Passo 3:** Substituir uso direto de `valor_inserido`:

```cpp
// ❌ ANTES (Não seguro)
void deposito_coin(int mensagem) {
  int valor = verifica_coin_tybe_deposited(mensagem);
  valor_inserido += valor;  // Pode ter race condition
  lcd.print(valor_inserido / 100);
}

// ✅ DEPOIS (Seguro)
void deposito_coin(int mensagem) {
  int valor = verifica_coin_tybe_deposited(mensagem);
  safe_valor_inserido.adicionar(valor);  // Operação atômica
  
  int total = safe_valor_inserido.ler();  // Leitura segura
  lcd.print(total / 100);
}
```

**Passo 4:** Em todos os lugares que usam `valor_inserido`:
```cpp
// Substituir:
if (valor_inserido >= preco_produto) {
  // ...
}

// Por:
int valor_atual = safe_valor_inserido.ler();
if (valor_atual >= preco_produto) {
  // ...
}
```

---

## 🟡 Correção 4: Proteção em Resets (Média Prioridade)

### Problema
Sistema pode resetar durante operação crítica.

### Solução Implementada
Verificar se é seguro resetar antes de executar reset.

### Código para Adicionar

**Passo 1:** Localizar código que faz reset (linhas 151, 170 do MDB.cpp):
```cpp
// ❌ ANTES (Reseta sem verificar)
if (sem_retorno_mdb == ATIVO) {
  Serial.println(F("MDB RESET"));
  unsigned long inicio_espera = millis();
  while(millis() - inicio_espera < 5000) {
    if (Serial1.available()) Serial1.read();
  }
  wdt_enable(WDTO_15MS);
  while(1) {}
}
```

**Passo 2:** Adicionar verificação:
```cpp
// ✅ DEPOIS (Verifica antes de resetar)
if (sem_retorno_mdb == ATIVO) {
  // Verifica se é seguro resetar
  if (pode_resetar_sistema(em_venda, controle_vmc)) {
    Serial.println(F("MDB RESET - System is safe to reset"));
    
    unsigned long inicio_espera = millis();
    while(millis() - inicio_espera < 5000) {
      if (Serial1.available()) Serial1.read();
    }
    
    wdt_enable(WDTO_15MS);
    while(1) {}
  } else {
    // Não pode resetar - tenta recuperar MDB sem reset total
    Serial.println(F("RESET BLOCKED - Trying MDB recovery"));
    mdb.reset();  // Apenas reset do MDB, não do sistema
    sem_retorno_mdb = INATIVO;
  }
}
```

---

## 📋 Checklist de Implementação

### Passo 1: Adicionar Arquivos
- [x] `CriticalFixes.h` criado
- [ ] Incluir `#include "CriticalFixes.h"` no `.ino`

### Passo 2: Timeout do Sensor
- [ ] Declarar `SensorTimeout sensor_timeout;`
- [ ] Chamar `sensor_timeout.start()` antes de ligar motor
- [ ] Adicionar verificação `sensor_timeout.timeout_occurred()` no loop de espera
- [ ] Implementar retorno de dinheiro em caso de timeout
- [ ] Chamar `sensor_timeout.stop()` ao detectar produto

### Passo 3: Persistência de Transação
- [ ] Declarar `TransactionManager transaction_manager;`
- [ ] Adicionar recuperação no `setup()`
- [ ] Chamar `transaction_manager.iniciar()` antes de ligar motor
- [ ] Chamar `transaction_manager.finalizar()` após sucesso/erro

### Passo 4: Race Conditions
- [ ] Declarar `SafeValueManager safe_valor_inserido;`
- [ ] Substituir `valor_inserido +=` por `safe_valor_inserido.adicionar()`
- [ ] Substituir leituras diretas por `safe_valor_inserido.ler()`

### Passo 5: Proteção em Reset
- [ ] Adicionar `pode_resetar_sistema()` antes de cada reset
- [ ] Implementar recovery alternativo (apenas MDB reset)

---

## 🧪 Testes Recomendados

### Teste 1: Timeout do Sensor
1. Iniciar venda normal
2. Inserir moeda e selecionar produto
3. **BLOQUEAR FISICAMENTE** o produto (não deixar cair)
4. Aguardar 10 segundos
5. **Esperado:** Sistema detecta timeout, retorna dinheiro, volta ao IDLE
6. **Verificar:** LCD mostra erro, log no Serial, troco é retornado

### Teste 2: Reset Durante Venda
1. Iniciar venda
2. Inserir R$10,00
3. Selecionar produto
4. **DESLIGAR ARDUINO** durante dispensa (antes do produto cair)
5. Religar Arduino
6. **Esperado:** Sistema detecta transação incompleta, retorna R$10,00
7. **Verificar:** Mensagem no Serial, troco é retornado

### Teste 3: Moedas Rápidas
1. Inserir 5 moedas rapidamente (< 1 segundo entre cada)
2. **Esperado:** Sistema conta todas corretamente
3. **Verificar:** Display mostra valor correto, log no Serial correto

### Teste 4: Tentativa de Reset em Venda
1. Iniciar venda (inserir moeda)
2. Forçar timeout do MDB (desconectar moedeiro)
3. **Esperado:** Sistema NÃO reseta, tenta recovery do MDB apenas
4. **Verificar:** Mensagem "RESET BLOCKED" no Serial

---

## 📊 Impacto das Correções

| Vulnerabilidade | Antes | Depois |
|-----------------|-------|--------|
| Timeout Sensor | Sistema trava | Retorna dinheiro em 10s ✅ |
| Reset em Venda | Perde R$10+ | Recupera e retorna ✅ |
| Race Conditions | Valores errados | Operações atômicas ✅ |
| Reset sem verificar | Reset sempre | Verifica segurança ✅ |

---

## ⚠️ IMPORTANTE

**BACKUP:** Antes de implementar, faça backup do código atual!

**TESTES:** Teste EXTENSIVAMENTE antes de colocar em produção!

**INCREMENTAL:** Implemente uma correção por vez e teste.

**LOG:** Monitore o Serial durante testes para ver mensagens de debug.

---

**Criado por:** GitHub Copilot  
**Data:** 2025-11-20  
**Versão:** 1.0  
**Status:** Pronto para implementação
