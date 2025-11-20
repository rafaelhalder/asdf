# Documentação Completa do Projeto - Máquina de Venda Automática (Vending Machine)

## 📋 Visão Geral do Sistema

Este é um firmware para Arduino Mega que controla uma **máquina de venda automática (vending machine)** que aceita moedas e notas através do protocolo MDB (Multi-Drop Bus). É como um servidor Node.js, mas rodando em hardware embarcado.

### Comparação com Node.js

| Aspecto | Node.js | Arduino (Este Projeto) |
|---------|---------|------------------------|
| **Execução** | Event Loop assíncrono | Loop infinito síncrono |
| **Memória** | ~2GB+ | ~8KB RAM |
| **Concorrência** | Async/Await, Promises | Polling com millis() |
| **I/O** | Arquivos, Rede, DB | GPIO, Serial, I2C, EEPROM |
| **Persistência** | Database (MongoDB, etc) | EEPROM (como LocalStorage) |

---

## 🏗️ Arquitetura do Sistema

### Componentes Principais

```
┌─────────────────────────────────────────────────┐
│           Arduino Mega (CPU Principal)          │
├─────────────────────────────────────────────────┤
│  ┌──────────────────────────────────────────┐  │
│  │  v018-valor-10-reais-novo-teclado.ino    │  │ ← Aplicação Principal
│  │  (Main Application - 4300+ linhas)       │  │
│  └──────────────────────────────────────────┘  │
│                      │                          │
│         ┌────────────┼────────────┐             │
│         ▼            ▼            ▼             │
│  ┌──────────┐ ┌──────────┐ ┌──────────────┐   │
│  │   MDB    │ │ Teclado  │ │ SensorQueda  │   │ ← Bibliotecas/Classes
│  │ (Moedas/ │ │ (Keypad) │ │   Infra      │   │
│  │  Notas)  │ │          │ │  (Detecção)  │   │
│  └──────────┘ └──────────┘ └──────────────┘   │
└─────────────────────────────────────────────────┘
         │             │              │
    ┌────┴────┐   ┌────┴───┐    ┌────┴────┐
    │ Serial1 │   │  GPIO  │    │ Serial3 │
    │ (MDB)   │   │ (Pinos)│    │(Sensor) │
    └─────────┘   └────────┘    └─────────┘
         │             │              │
    ┌────▼────┐   ┌────▼───┐    ┌────▼────┐
    │ Moedeiro│   │Teclado │    │ Sensor  │  ← Hardware Externo
    │   /     │   │  4x3   │    │  Queda  │
    │ Noteiro │   │        │    │ Produto │
    └─────────┘   └────────┘    └─────────┘
```

---

## 🔄 Fluxo de Execução (Como Node.js Event Loop, mas diferente)

### Setup (Inicialização - Como `app.listen()` no Express)

```cpp
void setup() {
  // 1. Inicializa comunicação serial (como abrir portas de rede)
  Serial.begin(115200);      // Debug (console.log)
  Serial1.begin(9600);       // MDB Protocol
  Serial3.begin(9600);       // Sensor
  
  // 2. Inicializa hardware (como conectar ao MongoDB)
  inicia_pinos();            // Configura GPIO
  inicializacao();           // Lê EEPROM (banco de dados local)
  
  // 3. Reset do MDB (como fazer um health check de API)
  mdb.reset();
}
```

### Loop (Ciclo Principal - Como Event Loop do Node.js)

```cpp
void loop() {
  // Em Node.js seria:
  // while(true) { processarEventos(); }
  
  sensor_queda_infra.task();    // Verifica sensor (como req.on('data'))
  task_controladora();          // Gerencia estado da máquina
  mostra_data();                // Atualiza LCD
  aguarda_inicializacao();      // Espera MDB ficar pronto
  
  if (inicializacao_ok && status_vmc) {
    mdb_task_main();            // CRÍTICO: Poll do MDB
  }
  
  statemachine_vmc();           // Máquina de estados (FSM)
  inatividade();                // Timeout/Watchdog
  buzzer_atv();                 // Feedback sonoro
  verifica_estoque();           // Verifica se tem produto
}
```

**⚠️ DIFERENÇA CRÍTICA DO NODE.JS:**
- No Node.js, `await fetch()` não bloqueia outras requisições
- No Arduino, `delay(5000)` **PARALISA TUDO** por 5 segundos
- Por isso usa `millis()` para timing não-bloqueante

---

## 🎯 Classes e Suas Responsabilidades

### 1. MDB (MDB.cpp/MDB.h) - "API Client" para Moedeiro/Noteiro

**Função:** Comunica com dispositivos MDB (aceitadores de moedas e notas)

**Em Node.js seria algo como:**
```javascript
class MDBClient {
  async poll() { /* Pinga dispositivos constantemente */ }
  async depositoCoin(valor) { /* Notifica moeda inserida */ }
  async depositoBill(valor) { /* Notifica nota inserida */ }
  async entregarTroco(valor) { /* Retorna troco */ }
}
```

**Protocolo MDB:**
- Como REST API, mas em serial com 9 bits
- Mestre (Arduino) envia comandos
- Escravos (moedeiro, noteiro) respondem
- **TIMING CRÍTICO:** Deve fazer poll a cada 100-200ms

**Principais Funções:**
```cpp
mdb.task()              // Gerencia comunicação MDB (como setInterval no Node)
mdb.deposito_coin(msg)  // Processa moeda inserida
mdb.deposito_bill(msg)  // Processa nota inserida
mdb.entregar_troco(val) // Retorna troco ao usuário
```

### 2. Teclado (Teclado.cpp/Teclado.h) - "Input Handler"

**Função:** Lê teclas pressionadas no teclado matricial 4x3

**Em Node.js seria:**
```javascript
class TecladoHandler {
  leitura() { 
    // Como req.body.key em Express
    return this.keypad.getKey(); 
  }
}
```

**Principais Funções:**
```cpp
teclado.leitura()       // Lê tecla (como readline no Node)
teclado.leitura_hold()  // Detecta tecla mantida pressionada
teclado.valor_lido()    // Retorna tempo que tecla foi pressionada
```

### 3. SensorQuedaInfra (SensorQuedaInfra.cpp/SensorQuedaInfra.h) - "Event Listener"

**Função:** Detecta se produto caiu após venda

**Em Node.js seria:**
```javascript
class SensorHandler {
  on('product_detected', () => {
    console.log('Produto entregue com sucesso!');
  });
}
```

**Principais Funções:**
```cpp
sensor.task()                  // Monitora sensor continuamente
sensor.get_evento_disponivel() // Verifica se produto caiu
sensor.realiza_leitura()       // Inicia detecção
sensor.finaliza_leitura()      // Para detecção
```

---

## 🔄 Máquina de Estados (FSM - Finite State Machine)

**Como funciona:** Semelhante a um `switch/case` em Node.js, mas gerencia o fluxo completo da venda.

```
Idle (Aguardando) 
    ↓ (moeda inserida)
Aguardando Seleção
    ↓ (tecla pressionada)
Verificando Crédito
    ↓ (crédito OK)
Liberando Produto
    ↓ (motor gira)
Aguardando Queda
    ↓ (sensor detecta)
Venda Completa → Idle
    OU
    ↓ (timeout sem detecção)
Retornando Troco → Idle
```

**Implementação:**
```cpp
void statemachine_vmc() {
  switch(controle_vmc) {
    case 0:  // Idle - aguardando inserção
      // Como servidor esperando requisição
      break;
      
    case 1:  // Moeda/nota inserida
      // Como req.body recebido
      valor_inserido += deposito;
      break;
      
    case 2:  // Aguardando seleção produto
      // Como aguardando user input
      if (tecla_pressionada) {
        // Verifica crédito
      }
      break;
      
    case 3:  // Liberando produto
      // Como processar pagamento
      digitalWrite(MOTOR, HIGH);
      break;
      
    case 4:  // Aguardando queda
      // Como aguardar confirmação
      if (sensor.get_evento_disponivel()) {
        // Sucesso!
        controle_vmc = 5;
      }
      break;
  }
}
```

---

## 💾 Persistência de Dados (EEPROM)

**EEPROM = LocalStorage do Arduino** (mas com limite de escritas!)

### Comparação:

| Node.js | Arduino (EEPROM) |
|---------|------------------|
| `localStorage.setItem('key', val)` | `EEPROM.write(addr, val)` |
| Sem limite de escritas | ~100.000 escritas por célula |
| Rápido | Lento (~3.3ms por byte) |

### Dados Armazenados:

```cpp
// Configuração (raramente alterado)
EEPROM_ADDR_FIRST_TIME         // Flag primeira execução
EEPROM_ADDR_TIPO_MAQUINA       // Tipo de máquina
EEPROM_ADDR_STATUS_VMC         // Status ativo/inativo

// Contabilidade (alterado em vendas)
EEPROM_ADDR_ESTOQUE            // Quantidade produtos
EEPROM_ADDR_VALOR_TOTAL        // Total inserido (histórico)
EEPROM_ADDR_RECEITA_TOTAL      // Receita total

// Eventos (log de falhas)
EEPROM_ADDR_QTD_EVENTOS_FALHA  // Contador de falhas
EEPROM_ADDR_DEZ_EVENTOS        // Últimos 10 eventos
```

### ⚠️ ANÁLISE DE FREQUÊNCIA DE ESCRITA:

**Bom (escreve raramente):**
- Primeira inicialização: 1x na vida
- Tipo de máquina: 1x ou manual
- Status VMC: Só quando liga/desliga

**Potencialmente Problemático:**
```cpp
// Grava a cada venda completa
escreve_eeprom(EEPROM_ADDR_RECEITA_TOTAL, receita);
```

**Cálculo:**
- 10 vendas/dia × 365 dias = 3.650 escritas/ano
- Limite: 100.000 escritas
- Vida útil: ~27 anos ✅ OK!

**Conclusão:** Uso de EEPROM está **adequado** - só grava em eventos importantes, não a cada loop.

---

## ⚠️ Problemas Identificados e Análise

### A. Uso Excessivo de Variáveis Globais

**Problema Identificado:** 97+ variáveis globais

**Em Node.js seria como:**
```javascript
// ❌ Ruim (como está agora)
let controle = 0;
let aux = 0;
let status_compra = false;
let valor_inserido = 0;
// ... 90+ mais

app.post('/venda', (req, res) => {
  controle = 1;  // Quem mudou isso???
  aux = controle;
});
```

**Solução Recomendada:**
```javascript
// ✅ Bom (encapsular em classes/structs)
class VMCState {
  int controle = 0;
  int valor_inserido = 0;
  bool status_compra = false;
}

VMCState vmc_state;
```

**Risco Real:**
- Difícil debugar (quem mudou `aux` ou `controle`?)
- Race conditions em sistemas mais complexos
- Em vending machine: pode entregar produto sem pagamento!

### B. Resets como Tratamento de Erro

**Código Atual:**
```cpp
if (sem_retorno_mdb == ATIVO) {
  Serial.println(F("MDB RESET"));
  delay(5000);
  wdt_enable(WDTO_15MS);  // RESET COMPLETO!
  while(1) {}
}
```

**Análise:**
- **Problema:** Esconde bugs reais
- **Cenário:** MDB não responde → Arduino reseta → perde histórico de erro
- **Solução Melhor:**
  ```cpp
  if (sem_retorno_mdb == ATIVO) {
    log_erro(ERR_MDB_TIMEOUT);  // Salvar erro
    mdb.reset();                // Resetar SÓ o MDB
    // NÃO resetar o Arduino inteiro
  }
  ```

### C. Comunicação Serial Bloqueante (Delays)

**Problema:** 25 chamadas `delay()` no código

**Exemplo Crítico:**
```cpp
// MDB.cpp linha 151
delay(5000);  // TRAVA TUDO POR 5 SEGUNDOS!
```

**Impacto:**
- Durante `delay(5000)`, o Arduino **não processa nada**
- MDB precisa de poll a cada 100-200ms
- Se demorar, moedeiro pode dar timeout

**Solução:**
```cpp
// ❌ Bloqueante
delay(5000);
asm volatile ("jmp 0");

// ✅ Não-bloqueante
unsigned long inicio = millis();
while(millis() - inicio < 5000) {
  mdb.task();  // Continua processando MDB
  // Outras tasks...
}
wdt_enable(WDTO_15MS);
```

### D. Frequência de Escrita EEPROM

**✅ ANÁLISE: Está OK!**

**Padrão de Uso:**
```cpp
// Escreve APENAS quando:
// 1. Venda finalizada com sucesso
// 2. Configuração manual alterada
// 3. Status liga/desliga mudado
```

**Não escreve a cada:**
- Loop (seria catastrófico!)
- Moeda inserida (só na finalização)
- Tecla pressionada

**Conclusão:** Uso adequado, sem risco de desgaste prematuro.

---

## 🔍 Fluxo Completo de Uma Venda

```
1. IDLE (Aguardando)
   └─> LCD: "Insira moedas/notas"

2. MOEDA INSERIDA (valor_inserido = 500 centavos)
   └─> mdb.deposito_coin() detecta
   └─> Atualiza LCD: "R$ 5,00"

3. SELEÇÃO DE PRODUTO
   └─> Usuário pressiona tecla "1"
   └─> teclado.leitura() retorna '1'
   └─> Verifica: valor_inserido >= preco_produto?

4. LIBERAÇÃO (Se crédito OK)
   └─> digitalWrite(MOTOR, HIGH)  // Liga motor
   └─> Produto começa a cair

5. DETECÇÃO DE QUEDA
   └─> sensor_queda_infra.task() monitora
   └─> Sensor detecta produto
   └─> sensor.get_evento_disponivel() == TRUE

6. FINALIZAÇÃO
   └─> Atualiza contadores:
       - estoque--
       - receita_total += preco_produto
   └─> Grava na EEPROM:
       escreve_eeprom(EEPROM_ADDR_RECEITA_TOTAL, receita)
   └─> LCD: "Obrigado!"
   └─> Volta para IDLE

--- FALHA? ---
Se produto não cai (timeout):
   └─> Retorna troco: mdb.entregar_troco(valor_inserido)
   └─> Log falha: qtd_eventos_falha++
   └─> Grava na EEPROM
```

---

## 📊 Timing e Performance

### Ciclo do Loop Principal

```
┌─────────────────────────────────────┐
│  Loop: ~10-50ms por ciclo           │
├─────────────────────────────────────┤
│  sensor_queda_infra.task()  ~1ms    │
│  task_controladora()        ~2ms    │
│  mostra_data()              ~5ms    │
│  mdb_task_main()            ~10ms   │  ← CRÍTICO
│  statemachine_vmc()         ~2ms    │
│  buzzer_atv()               ~1ms    │
└─────────────────────────────────────┘
```

**MDB Timing (CRÍTICO):**
- Poll deve ocorrer a cada 100-200ms
- Se atrasar, moedeiro pode dar timeout
- `delay()` é **mortal** para esse timing!

### Uso de Memória (RAM)

- Arduino Mega: 8KB RAM total
- Variáveis globais: ~3-4KB estimado
- Stack: ~1KB
- Buffers Serial: 256 bytes
- **Restante: ~3KB livre** (relativamente OK)

---

## 🛠️ Melhorias Recomendadas (Roadmap)

### Prioridade Alta (Críticas)

1. **Eliminar `delay()` em contextos críticos**
   ```cpp
   // Substituir todos delay() por millis()
   unsigned long timer = millis();
   while(millis() - timer < 5000) {
     mdb.task();  // Mantém MDB ativo
   }
   ```

2. **Encapsular variáveis globais**
   ```cpp
   struct VMCState {
     int controle;
     int valor_inserido;
     bool em_venda;
   } vmc;
   
   // Usar: vmc.controle ao invés de controle global
   ```

3. **Logging de erros persistente**
   ```cpp
   void log_erro(int erro_code) {
     // Salvar timestamp + código erro
     // Não apenas resetar e esquecer
   }
   ```

### Prioridade Média

4. **Watchdog Timer ativo** ✅ (JÁ FEITO!)

5. **Reduzir uso de String (Arduino)**
   ```cpp
   // ❌ Evitar (fragmenta heap)
   String comando = "teste";
   
   // ✅ Preferir
   char comando[20] = "teste";
   ```

6. **State Machine mais robusta**
   ```cpp
   enum VMCState {
     IDLE,
     AGUARDANDO_SELECAO,
     PROCESSANDO_VENDA,
     LIBERANDO_PRODUTO
   };
   VMCState estado = IDLE;
   ```

### Prioridade Baixa (Nice to Have)

7. Adicionar CRC/Checksum em comunicação MDB
8. Implementar retry logic (ao invés de reset)
9. Telemetria (enviar status via Serial para supervisão)

---

## 📚 Glossário Técnico

| Termo | Significado | Equivalente Node.js |
|-------|-------------|---------------------|
| **Loop** | Ciclo infinito principal | Event Loop |
| **Poll** | Verificar estado periodicamente | setInterval() |
| **millis()** | Tempo desde boot (ms) | Date.now() |
| **delay()** | Pausa bloqueante | setTimeout() (mas BLOQUEANTE) |
| **GPIO** | Pinos digitais/analógicos | N/A (hardware) |
| **Serial** | Comunicação serial (UART) | Socket/Serial Port |
| **EEPROM** | Memória persistente | localStorage/Database |
| **I2C** | Protocolo comunicação (RTC) | N/A (hardware) |
| **FSM** | Finite State Machine | switch/case com estado |
| **Watchdog** | Timer que reseta se travou | pm2 auto-restart |

---

## 🎓 Conceitos Importantes para Quem Vem de Node.js

### 1. Não Há "Async/Await"
```javascript
// Node.js
async function venda() {
  const pagamento = await processarPagamento();
  const produto = await liberarProduto();
}

// Arduino (tudo é síncrono/polling)
void venda() {
  if (pagamento_completo) {
    liberar_produto();
  }
}
```

### 2. Memória É Escassa
```javascript
// Node.js: Array gigante? Sem problema!
const historico = new Array(1000000);

// Arduino: 8KB RAM total!
int historico[10];  // Apenas 10 itens
```

### 3. Não Há File System
```javascript
// Node.js
fs.writeFile('vendas.json', data);

// Arduino
EEPROM.write(addr, data);  // Apenas 4KB, lento, limite de escritas
```

### 4. Timing É Manual
```javascript
// Node.js (event-driven)
setTimeout(() => resetMDB(), 5000);
// Código continua rodando...

// Arduino (polling)
unsigned long inicio = millis();
while(millis() - inicio < 5000) {
  // Processar outras coisas manualmente
}
```

---

## 📝 Resumo Executivo

**O Que o Projeto Faz:**
Controla uma máquina de venda automática que:
- Aceita moedas e notas (protocolo MDB)
- Permite seleção via teclado matricial
- Libera produtos através de motor
- Detecta se produto caiu (sensor infravermelho)
- Retorna troco quando necessário
- Persiste dados de venda em EEPROM
- Gerencia display LCD para interação

**Principais Desafios:**
1. ✅ Timing crítico do protocolo MDB (resolvido com polling)
2. ⚠️ Delays bloqueantes podem causar timeouts MDB
3. ⚠️ 97 variáveis globais dificultam manutenção
4. ✅ EEPROM usado adequadamente (sem desgaste excessivo)
5. ✅ Reset agora usa watchdog (método seguro)

**Estado Atual:**
- ✅ Funcional e estável
- ⚠️ Código precisa refatoração (globals, delays)
- ✅ Sem riscos de segurança críticos
- ✅ Performance adequada para vending machine

---

**Documentação criada por:** GitHub Copilot  
**Data:** 2025-11-20  
**Versão:** 1.0
