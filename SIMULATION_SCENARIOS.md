# Simulações de Cenários - Testes de Comportamento do Sistema

## 📋 Objetivo

Este documento apresenta **simulações detalhadas** de diversos cenários para identificar **possíveis furos/vulnerabilidades** no código da máquina de venda automática.

---

## 🎯 Cenário 1: Ataque de Moedas Rápidas (Race Condition)

### Descrição
Usuário insere múltiplas moedas rapidamente para tentar confundir o sistema.

### Código Envolvido
```cpp
// v018-valor-10-reais-novo-teclado.ino
int valor_inserido = 0;  // GLOBAL

// MDB.cpp
void deposito_coin(int mensagem) {
  int valor = verifica_coin_tybe_deposited(mensagem);
  // ... processamento ...
}
```

### Simulação Passo a Passo

**T=0ms:** Sistema em IDLE, `valor_inserido = 0`

**T=10ms:** Moeda de R$1,00 detectada
```cpp
// MDB detecta moeda
deposito_coin(0x02);  // Código para R$1,00
// valor_inserido += 100;  (centavos)
valor_inserido = 100;
```

**T=15ms:** LCD atualiza: "R$ 1,00"

**T=20ms:** Segunda moeda de R$1,00 inserida (RÁPIDO!)
```cpp
deposito_coin(0x02);
valor_inserido = 200;  // Correto: 100 + 100
```

**T=25ms:** Terceira moeda enquanto LCD ainda atualiza
```cpp
deposito_coin(0x02);
valor_inserido = 300;  // OK ainda
```

### 🔴 VULNERABILIDADE IDENTIFICADA

**Cenário de Falha:**
Se o código usar `valor_inserido` em múltiplos lugares **sem sincronização**, pode ocorrer:

```cpp
// Thread 1: Atualizando LCD
void mostra_valor() {
  int temp = valor_inserido;  // Lê 200
  // ... delay para atualizar LCD ...
  lcd.print(temp/100);        // Mostra 2
}

// Thread 2: Recebendo nova moeda
void deposito_coin() {
  valor_inserido += 100;      // Agora é 300!
}

// Resultado: LCD mostra R$2,00 mas sistema tem R$3,00
```

**RISCO:** Usuário vê valor menor no display mas sistema cobra o correto (ou vice-versa).

### ✅ Mitigação Atual
Arduino é **single-threaded**, mas interrupts podem causar problema similar.

**Solução Recomendada:**
```cpp
// Usar variável volátil
volatile int valor_inserido = 0;

// Desabilitar interrupts durante operações críticas
noInterrupts();
int temp = valor_inserido;
valor_inserido += novo_valor;
interrupts();
```

---

## 🎯 Cenário 2: Produto Não Cai - Usuário Perde Dinheiro

### Descrição
Usuário paga, motor gira, mas produto não cai do dispensador.

### Código Envolvido
```cpp
// statemachine_vmc()
case 3:  // LIBERANDO PRODUTO
  digitalWrite(MOTOR, HIGH);
  controle_vmc = 4;  // Vai para AGUARDANDO_QUEDA
  break;

case 4:  // AGUARDANDO QUEDA
  if (sensor.get_evento_disponivel()) {
    // Produto caiu!
    controle_vmc = 5;
  }
  // ⚠️ MAS E SE NÃO CAI?
  break;
```

### Simulação Passo a Passo

**T=0s:** Usuário insere R$5,00
- `valor_inserido = 500`

**T=5s:** Usuário seleciona produto (tecla "1")
- Preço: R$5,00
- `controle_vmc = 3` (Liberar produto)

**T=6s:** Motor liga
- `digitalWrite(MOTOR, HIGH)`
- `controle_vmc = 4` (Aguardar queda)

**T=7s:** Motor gira por 3 segundos
- Produto **TRAVA** no dispensador

**T=10s:** Sensor não detecta nada
- `sensor.get_evento_disponivel() = 0`
- Sistema **FICA PRESO** no estado 4!

**T=30s:** Usuário desiste, perdeu R$5,00

### 🔴 VULNERABILIDADE IDENTIFICADA

**Problema:** Falta **TIMEOUT** no estado de aguardar queda!

**Código Atual:**
```cpp
case 4:  // AGUARDANDO QUEDA
  if (sensor.get_evento_disponivel()) {
    // Sucesso
  }
  // ⚠️ NENHUM TIMEOUT!
  // Sistema fica travado aqui para sempre
}
```

### ✅ Mitigação Necessária

**Solução:**
```cpp
// Adicionar timeout
unsigned long timeout_queda = 0;

case 3:  // LIBERANDO PRODUTO
  digitalWrite(MOTOR, HIGH);
  timeout_queda = millis();
  controle_vmc = 4;
  break;

case 4:  // AGUARDANDO QUEDA
  if (sensor.get_evento_disponivel()) {
    // Sucesso!
    receita_total += preco_produto;
    controle_vmc = 5;
  } 
  else if (millis() - timeout_queda > 10000) {  // 10s timeout
    // FALHA: Produto não caiu
    digitalWrite(MOTOR, LOW);
    
    // Registra erro
    errorLog.logError(ERR_SENSOR_TIMEOUT, produto_selecionado);
    
    // Retorna dinheiro
    mdb.entregar_troco(valor_inserido);
    valor_inserido = 0;
    
    // Log na EEPROM
    qtd_eventos_falha++;
    
    controle_vmc = 0;  // Volta para IDLE
  }
  break;
```

---

## 🎯 Cenário 3: Reset Durante Venda - Perda de Estado

### Descrição
Sistema reseta (watchdog ou erro) durante uma venda em andamento.

### Simulação Passo a Passo

**T=0s:** Usuário insere R$10,00
- `valor_inserido = 1000`
- Dados na **RAM** (volátil)

**T=5s:** Usuário seleciona produto de R$8,00
- `controle_vmc = 3`
- Motor começa a girar

**T=6s:** **MDB TIMEOUT** - Sistema reseta
```cpp
// MDB.cpp linha 150
if (sem_retorno_mdb == ATIVO) {
  errorLog.logError(ERR_MDB_TIMEOUT);
  wdt_enable(WDTO_15MS);
  while(1) {}  // RESET!
}
```

**T=6.015s:** Sistema reinicia
- `valor_inserido = 0` (RAM zerada!)
- `controle_vmc = 0`
- Motor para

**T=10s:** Usuário vê:
- Produto não saiu
- Dinheiro sumiu
- Display zerado

### 🔴 VULNERABILIDADE IDENTIFICADA

**Problema:** Estado de venda não é persistido!

**Perda de Dados:**
- `valor_inserido` (RAM)
- `controle_vmc` (RAM)
- `produto_selecionado` (RAM)

**IMPACTO:** Usuário perde R$10,00!

### ✅ Mitigação Necessária

**Solução 1: Persistir Estado Crítico**
```cpp
// Antes de resetar
struct TransacaoEmAndamento {
  bool ativa;
  int valor_inserido;
  int produto_selecionado;
  unsigned long timestamp;
};

#define EEPROM_TRANSACAO_ADDR 1600

void salvar_transacao() {
  if (em_venda) {
    TransacaoEmAndamento t;
    t.ativa = true;
    t.valor_inserido = valor_inserido;
    t.produto_selecionado = posicao;
    t.timestamp = millis();
    EEPROM.put(EEPROM_TRANSACAO_ADDR, t);
  }
}

// No setup, após reset
void recuperar_transacao() {
  TransacaoEmAndamento t;
  EEPROM.get(EEPROM_TRANSACAO_ADDR, t);
  
  if (t.ativa) {
    // Transação incompleta detectada!
    Serial.println(F("Transação incompleta recuperada"));
    
    // Retornar dinheiro através do MDB
    mdb.entregar_troco(t.valor_inserido);
    
    // Limpar
    t.ativa = false;
    EEPROM.put(EEPROM_TRANSACAO_ADDR, t);
    
    // Log
    errorLog.logError(ERR_GENERIC, ERR_TRANSACAO_INTERROMPIDA);
  }
}
```

**Solução 2: Evitar Reset Durante Venda**
```cpp
bool pode_resetar() {
  return !em_venda && controle_vmc == 0;
}

if (sem_retorno_mdb == ATIVO && pode_resetar()) {
  // OK resetar
} else {
  // Tentar recuperar MDB sem resetar
  mdb.reset();
}
```

---

## 🎯 Cenário 4: Buffer Overflow no Sensor Serial

### Descrição
Sensor envia dados corrompidos ou ataque proposital.

### Código Envolvido
```cpp
// SensorQuedaInfra.cpp
void serialEvent() {
  while(Serial3.available()) {
    char inChar = (char)Serial3.read();
    if (inChar == '\n') {
      stringComplete = true;
    } else if (string_serial.length() < 999) {  // ✅ JÁ CORRIGIDO!
      string_serial += inChar;
    }
  }
}
```

### Simulação de Ataque

**T=0ms:** Sensor começa a enviar dados
```
"SENSOR:OK\n" (normal)
```

**T=10ms:** Sistema recebe e processa
- `string_serial = "SENSOR:OK"`
- Tudo OK

**T=20ms:** Sensor com defeito envia lixo
```
"AAAAAAAAAAAAAAAAAAAA..." (sem '\n')
```

**T=30ms:** String cresce
- `string_serial.length() = 100`
- `string_serial.length() = 200`
- `string_serial.length() = 500`

**T=50ms:** Tentativa de overflow
- `string_serial.length() = 998`
- Próximo char: **BLOQUEADO** por `if (length < 999)`

### ✅ Já Mitigado!

O código **já foi corrigido** com proteção:
```cpp
if (string_serial.length() < 999) {
  string_serial += inChar;
}
```

**Antes da correção:** Sistema crashava com buffer overflow.
**Depois da correção:** String para em 999 caracteres.

---

## 🎯 Cenário 5: Variável Global Corrompida (Uso Ambíguo)

### Descrição
Variável `controle` usada em múltiplos contextos.

### Código Problemático
```cpp
// GLOBAL
int controle = 0;  // ⚠️ Uso ambíguo!

// Função A
void funcao_menu() {
  controle = 1;  // Marca "em menu"
}

// Função B
void funcao_venda() {
  if (controle == 0) {  // Verifica "sistema livre"
    iniciar_venda();
  }
}

// Função C  
void funcao_teste() {
  controle = 5;  // Marca "em teste"
}
```

### Simulação de Bug

**T=0s:** Sistema em menu
- `controle = 1`

**T=5s:** Usuário sai do menu
- `controle = 0`

**T=6s:** Função de teste automático roda
- `controle = 5`

**T=7s:** Usuário tenta comprar
```cpp
funcao_venda() {
  if (controle == 0) {  // ❌ FALSO! controle = 5
    iniciar_venda();    // Nunca executa!
  }
}
```

**T=10s:** Usuário não consegue comprar!

### 🔴 VULNERABILIDADE IDENTIFICADA

**Problema:** Variável `controle` tem **múltiplos significados**.

**Situações de Risco:**
1. `controle` é alterado por função não relacionada
2. Valores colidem (ex: menu=1, erro=1)
3. Difícil debugar

### ✅ Mitigação com Structs (Já Disponível)

```cpp
// Usar VMCState.h
VMCState vmc;

vmc.controle = 1;       // Controle do VMC
vmc.controle_vmc = 2;   // Estado da máquina de estados

// Agora é CLARO qual controle está sendo usado
```

---

## 🎯 Cenário 6: EEPROM Write Amplification (Desgaste Prematuro)

### Descrição
Sistema grava EEPROM com muita frequência.

### Análise de Frequência

**Operação:** Atualizar `receita_total` a cada moeda

```cpp
// ❌ ERRADO (hipotético)
void deposito_coin() {
  valor_inserido += 100;
  receita_total += 100;
  escreve_eeprom(EEPROM_ADDR_RECEITA_TOTAL_1, 
                 EEPROM_ADDR_RECEITA_TOTAL_2, 
                 receita_total);  // GRAVA A CADA MOEDA!
}
```

**Simulação:**
- 100 moedas/dia
- 365 dias/ano
- **36.500 gravações/ano**

**Vida Útil EEPROM:** 100.000 gravações
- **2,7 anos** até falha!

### ✅ Código Atual Está SEGURO

```cpp
// ✅ CORRETO (código atual)
// Grava APENAS ao finalizar venda
case 5:  // VENDA_COMPLETA
  receita_total += preco_produto;
  escreve_eeprom(EEPROM_ADDR_RECEITA_TOTAL_1, 
                 EEPROM_ADDR_RECEITA_TOTAL_2, 
                 receita_total);  // 1x por venda
  break;
```

**Frequência Real:**
- 10 vendas/dia (não 100 moedas)
- **3.650 gravações/ano**
- Vida útil: **27 anos** ✅

---

## 🎯 Cenário 7: Deadlock no Delay Bloqueante (JÁ CORRIGIDO)

### Descrição
MDB para de responder durante delay.

### Código Antigo (VULNERÁVEL)
```cpp
// ❌ ANTES
void setup_mdb() {
  mdb_envia(comando);
  delay(5000);  // TRAVA TUDO!
  le_resposta();
}
```

### Simulação de Deadlock

**T=0s:** MDB setup inicia
- Envia comando
- `delay(5000)` começa

**T=1s:** Moedeiro tenta enviar poll
- Sistema **NÃO RESPONDE** (em delay)

**T=3s:** Moedeiro timeout
- Desconecta

**T=5s:** Delay termina
- Tenta ler resposta
- **NADA** (moedeiro já desconectou)

**T=6s:** Sistema reseta por erro MDB

### ✅ Já Corrigido!

```cpp
// ✅ DEPOIS
void setup_mdb() {
  mdb_envia(comando);
  
  unsigned long inicio = millis();
  while(millis() - inicio < 5000) {
    // Sistema continua responsivo!
    if (Serial1.available()) {
      processar_mdb();
    }
  }
  
  le_resposta();
}
```

---

## 🎯 Cenário 8: Integer Overflow em Receita

### Descrição
Receita ultrapassa limite de `long`.

### Código Envolvido
```cpp
long receita_total;  // long = 32 bits = -2B a +2B
```

### Simulação

**Capacidade:**
```
long max = 2.147.483.647 centavos
         = 21.474.836 reais
```

**Cenário:**
- Máquina vende R$1.000/dia
- Tempo para overflow: **58 anos**

### ✅ Não É Problema Prático

Para máquinas normais, `long` é suficiente.

**Se necessário (máquina de alto volume):**
```cpp
// Usar unsigned long long (64 bits)
unsigned long long receita_total;  // 0 a 18 quintilhões
```

---

## 🎯 Cenário 9: Timing Attack no Teclado

### Descrição
Atacante mede tempo de resposta para descobrir PIN.

### Código Vulnerável (Hipotético)
```cpp
// ❌ VULNERÁVEL
bool verifica_pin(char* input) {
  char pin[] = "1234";
  for(int i = 0; i < 4; i++) {
    if (input[i] != pin[i]) {
      return false;  // ⚠️ Retorna CEDO!
    }
  }
  return true;
}

// Timing:
// "0000" -> retorna em 1µs (primeira letra errada)
// "1000" -> retorna em 2µs (segunda letra errada)
// "1200" -> retorna em 3µs (terceira letra errada)
// "1230" -> retorna em 4µs (quarta letra errada)
// "1234" -> retorna em 5µs (todas corretas)
```

**Atacante pode MEDIR o tempo e descobrir PIN!**

### ✅ Mitigação

```cpp
// ✅ SEGURO
bool verifica_pin_safe(char* input) {
  char pin[] = "1234";
  bool correto = true;
  
  // Verifica TODAS as posições (tempo constante)
  for(int i = 0; i < 4; i++) {
    if (input[i] != pin[i]) {
      correto = false;
      // NÃO retorna! Continua verificando
    }
  }
  
  return correto;
}
```

---

## 📊 Resumo de Vulnerabilidades Identificadas

| # | Cenário | Severidade | Status |
|---|---------|------------|--------|
| 1 | Race Condition (Moedas Rápidas) | 🟡 Média | ⚠️ Mitigar |
| 2 | Timeout Sensor (Produto Não Cai) | 🔴 Alta | ⚠️ **CRÍTICO** |
| 3 | Reset Durante Venda | 🔴 Alta | ⚠️ **CRÍTICO** |
| 4 | Buffer Overflow Serial | 🟢 Baixa | ✅ Corrigido |
| 5 | Variável Global Ambígua | 🟡 Média | ⚠️ Melhorar |
| 6 | EEPROM Wear | 🟢 Baixa | ✅ OK |
| 7 | Deadlock Delay | 🟢 Baixa | ✅ Corrigido |
| 8 | Integer Overflow | 🟢 Baixa | ✅ OK |
| 9 | Timing Attack | 🟡 Média | ℹ️ Se usar PIN |

---

## 🔧 Recomendações de Correção Prioritárias

### 1. CRÍTICO: Adicionar Timeout no Sensor de Queda
```cpp
// No statemachine_vmc(), case 4
if (millis() - timeout_queda > 10000) {
  errorLog.logError(ERR_SENSOR_TIMEOUT);
  mdb.entregar_troco(valor_inserido);
  controle_vmc = 0;
}
```

### 2. CRÍTICO: Persistir Transações Incompletas
```cpp
// Salvar antes de operações arriscadas
void salvar_checkpoint() {
  if (em_venda) {
    EEPROM.put(EEPROM_TRANSACAO_ADDR, transacao_atual);
  }
}
```

### 3. IMPORTANTE: Usar Structs ao Invés de Globais
```cpp
// Migrar gradualmente para VMCState
VMCState vmc_state;
vmc_state.controle_vmc = 1;  // Mais claro
```

---

## ✅ Conclusão

**Vulnerabilidades Críticas Encontradas:** 2
- Timeout do sensor
- Perda de estado em reset

**Vulnerabilidades Já Corrigidas:** 3
- Buffer overflow
- Delays bloqueantes
- EEPROM wear

**Código Geral:** 🟡 Bom, mas requer melhorias críticas

**Próximos Passos:**
1. Implementar timeout no sensor (URGENTE)
2. Adicionar persistência de transação (URGENTE)
3. Continuar migração para structs (Médio prazo)

---

**Criado por:** GitHub Copilot  
**Data:** 2025-11-20  
**Versão:** 1.0  
**Tipo:** Análise de Segurança e Simulações
