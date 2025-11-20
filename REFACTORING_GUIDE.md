# Guia de Refatoração - Melhorias Críticas Identificadas

## 🎯 Objetivo

Este documento identifica os problemas críticos do código atual e fornece um plano detalhado de refatoração para melhorar:
1. **Manutenibilidade**: Reduzir variáveis globais
2. **Confiabilidade**: Eliminar delays bloqueantes
3. **Segurança**: Evitar race conditions e bugs financeiros

---

## ⚠️ Problema 1: Excesso de Variáveis Globais (97+)

### Situação Atual

```cpp
// v018-valor-10-reais-novo-teclado.ino (linhas 107-200)
int controle = 0;
int aux = 0;
int controle_vmc = 0;
int valor_inserido = 0;
bool em_venda = 0;
bool status_compra = 0;
// ... + 90 variáveis mais
```

### Por Que É Problemático?

```cpp
// ❌ Cenário de Bug Real:
void funcao_a() {
  controle = 1;  // Quem mudou isso? Quando? Por quê?
}

void funcao_b() {
  if (controle == 1) {  // Pode estar em estado inesperado!
    liberar_produto();  // PERIGO: Produto de graça!
  }
}
```

### Solução Recomendada: Encapsulamento em Structs

```cpp
// ✅ Estrutura Proposta:

// 1. Estado da Máquina Vending (VMC)
struct VMCState {
  int controle;
  int controle_vmc;
  int valor_inserido;
  int valor_inserido_total;
  bool em_venda;
  bool status_compra;
  int aux;
  int posicao;
} vmc;

// 2. Estado do MDB (comunicação com moedeiro/noteiro)
struct MDBState {
  bool boot_mdb;
  bool inicializacao_ok;
  int controle_bill;
  int contador_bill;
  int escrow_ativo;
  int valor_inserido_bill_escrow;
  int valor_inserido_bill;
  int controle_deposito_mdb;
  int mdb_task_ctl;
  bool mdb_bill_sem_atividade;
  bool aguarda_reset_bill;
  int type_escrow_1;
  int type_escrow_2;
  int bill_type_deposited[5];
  int bill_routing[3];
  int dado_poll[10];
} mdb_vars;

// 3. Estado de Vendas e Estoque
struct SalesState {
  long estoque;
  long valor_total_inserido;
  long i_valor_total_inserido;
  long receita_total;
  long i_receita_total;
  int ultimo_valor_inserido;
  int qtd_eventos_falha;
  int controle_dez_eventos;
} vendas;

// 4. Estado do Display
struct DisplayState {
  bool pisca_pontos;
  int controle_visualiza;
  int mostra_msg_ini;
  int linha_ini;
  unsigned short int hora_1, hora_2;
  unsigned short int minuto_1, minuto_2;
  unsigned short int dia_1, dia_2;
  unsigned short int mes_1, mes_2;
  unsigned short int ano_1, ano_2;
} display;

// 5. Estado do Hardware
struct HardwareState {
  int controle_ldr;
  int ldr_max;
  int contador_moedas;
  int qtd_moedas_dispensar;
  long timeout_motor;
  int controle_timeout_motor;
  bool teste_entrega;
  int controle_buzzer;
} hardware;

// 6. Configuração do Sistema
struct SystemConfig {
  int first_time;
  int tipo_maquina;
  int status_maquina;
  int em_inicializacao;
  int altera_estado_notas;
  long pin_parte[6];
  unsigned int config_preco_valor[10];
} config;
```

### Benefícios

```cpp
// Antes (confuso):
controle_vmc = 1;
controle = 2;
aux = controle;  // Qual "controle"?

// Depois (claro):
vmc.controle_vmc = 1;
vmc.controle = 2;
vmc.aux = vmc.controle;  // Óbvio que é o controle do VMC
```

### Plano de Migração Gradual

**Fase 1: Definir Structs (Sem Quebrar Código)**
```cpp
// Definir structs mas manter variáveis globais existentes
struct VMCState {
  int *controle;  // Ponteiro para a global existente
  int *controle_vmc;
  // ...
} vmc;

// Inicializar ponteiros
void init_structs() {
  vmc.controle = &controle;
  vmc.controle_vmc = &controle_vmc;
}

// Agora pode usar: *vmc.controle ou controle (ambos funcionam)
```

**Fase 2: Substituir Uso Gradualmente**
```cpp
// Ir trocando aos poucos:
if (controle_vmc == 1)     →   if (vmc.controle_vmc == 1)
```

**Fase 3: Remover Globais**
```cpp
// Quando todas referências forem atualizadas, remover globais
```

---

## ⚠️ Problema 2: Delays Bloqueantes

### Situação Atual

```cpp
// MDB.cpp - Localização dos Delays Críticos
Linha 151:  delay(5000);  // 5 segundos PARADO antes de reset
Linha 170:  delay(5000);  // 5 segundos PARADO antes de reset
Linha 584:  delay(50);    // Durante inicialização
Linha 1005: delay(100);   // Durante setup MDB
Linha 1017: delay(10);    // Loop de leitura (condicional)
// ... + 20 delays de 10-30ms
```

### Por Que É Problemático?

```cpp
// ❌ Durante delay(), o Arduino NÃO processa:
delay(5000);  // Por 5 segundos:
              // - MDB poll NÃO acontece (pode dar timeout!)
              // - Sensor NÃO é lido
              // - LCD NÃO atualiza
              // - Botões NÃO respondem
              // Sistema está "MORTO"

// Protocolo MDB requer:
// - Poll a cada 100-200ms (máximo!)
// - Se delay(5000), perde 25 polls!
// - Moedeiro pode desconectar
```

### Análise de Criticidade

| Linha | Delay | Contexto | Criticidade |
|-------|-------|----------|-------------|
| 151, 170 | 5000ms | Antes de reset sistema | ⚠️ Média (sistema vai resetar mesmo) |
| 1005 | 100ms | Setup inicial MDB | 🔴 Alta (pode perder resposta) |
| 584, 617, 715 | 10-50ms | Loops de leitura | 🟡 Média (múltiplos podem somar) |
| 1196, 1220, etc | 15-30ms | Envio comandos MDB | 🟢 Baixa (necessário para timing) |

### Solução: Delays Não-Bloqueantes com millis()

#### Padrão Recomendado

```cpp
// ❌ ERRADO (Bloqueante):
void aguardar_resposta() {
  mdb_envia_comando();
  delay(100);  // TRAVA TUDO!
  le_resposta();
}

// ✅ CORRETO (Não-bloqueante):
unsigned long timer_resposta = 0;
bool aguardando_resposta = false;

void aguardar_resposta_nb() {
  if (!aguardando_resposta) {
    mdb_envia_comando();
    timer_resposta = millis();
    aguardando_resposta = true;
  }
  
  // No loop principal:
  if (aguardando_resposta && millis() - timer_resposta >= 100) {
    le_resposta();
    aguardando_resposta = false;
  }
  
  // Enquanto aguarda, outras tasks continuam rodando!
}
```

#### Exemplo Prático: Substituir delay(5000) antes de reset

```cpp
// ❌ ANTES (MDB.cpp linha 150-153):
Serial.println(F("MDB RESET"));
delay(5000);  // TRAVA POR 5 SEGUNDOS!
wdt_enable(WDTO_15MS);
while(1) {}

// ✅ DEPOIS (Não-bloqueante):
Serial.println(F("MDB RESET - Aguardando 5s..."));
unsigned long inicio_espera = millis();
while(millis() - inicio_espera < 5000) {
  // Continua processando durante a espera
  mdb.task();           // Mantém MDB vivo
  sensor.task();        // Continua monitorando sensor
  // ... outras tasks críticas
  
  // Feedback visual
  if ((millis() - inicio_espera) % 1000 == 0) {
    Serial.print(".");
  }
}
Serial.println(F("\nResetando agora!"));
wdt_enable(WDTO_15MS);
while(1) {}
```

#### Exemplo: Substituir delay(100) durante MDB setup

```cpp
// ❌ ANTES (MDB.cpp linha 1005):
for(int cont=0; cont < 3; cont++) {
  mdb_envia(data[cont]);   
}
delay(100);  // BLOQUEANTE!
for(int i = 0; i < 33; i++) {
  if(Serial1.available()) {
    data[i] = Serial1.read();
  }
}

// ✅ DEPOIS (Timeout com processamento):
for(int cont=0; cont < 3; cont++) {
  mdb_envia(data[cont]);   
}

// Aguarda resposta com timeout não-bloqueante
unsigned long inicio = millis();
int i = 0;
while(i < 33 && millis() - inicio < 100) {
  if(Serial1.available()) {
    data[i] = Serial1.read();
    i++;
  }
  // Loop vazio aguarda, mas pode processar outras coisas se necessário
}

// Verifica se recebeu tudo
if (i < 33) {
  Serial.println(F("Timeout aguardando resposta MDB"));
}
```

### Delays Que DEVEM Permanecer

```cpp
// ✅ Alguns delays são necessários para timing do protocolo MDB:
mdb_envia(comando);
delay(15);  // MDB requer 15ms entre comandos
mdb_envia(proximo_comando);

// Estes são OK porque:
// 1. São curtos (15-30ms)
// 2. São requeridos pelo protocolo
// 3. Não afetam significativamente o polling
```

---

## 📋 Plano de Implementação Recomendado

### Prioridade 1: Crítico (Fazer Primeiro) 🔴

1. **Substituir delay(5000) por espera não-bloqueante**
   - Arquivos: MDB.cpp linhas 151, 170
   - Impacto: Baixo risco (antes de reset)
   - Tempo: 30 minutos

2. **Documentar todas variáveis globais com comentários**
   - Arquivo: v018-valor-10-reais-novo-teclado.ino
   - Adicionar comentário explicando propósito de cada uma
   - Tempo: 1 hora

3. **Criar REFACTORING_ROADMAP.md** ✅ (Este arquivo!)
   - Documenta plano completo
   - Tempo: 1 hora

### Prioridade 2: Importante (Fazer em Seguida) 🟡

4. **Substituir delay(100) em MDB setup**
   - Arquivo: MDB.cpp linha 1005
   - Usar timeout com while()
   - Tempo: 1 hora

5. **Definir structs (sem quebrar código)**
   - Criar arquivo VMCState.h com definições
   - Não alterar código ainda, só preparar
   - Tempo: 2 horas

6. **Adicionar sistema de logging de erros**
   - Salvar últimos 10 erros na EEPROM
   - Evitar reset sem registro
   - Tempo: 3 horas

### Prioridade 3: Melhorias (Longo Prazo) 🟢

7. **Migrar variáveis para structs gradualmente**
   - Uma struct por vez (começar com VMCState)
   - Teste extensivo a cada migração
   - Tempo: 1-2 semanas

8. **Implementar State Machine com enums**
   ```cpp
   enum VMCState {
     IDLE,
     AGUARDANDO_SELECAO,
     PROCESSANDO_VENDA,
     LIBERANDO_PRODUTO,
     AGUARDANDO_QUEDA
   };
   ```
   - Tempo: 1 semana

9. **Adicionar telemetria via Serial**
   - Enviar status para supervisão externa
   - Tempo: 1 semana

---

## 🔧 Ferramentas e Helpers Recomendados

### Helper: Delay Não-Bloqueante

```cpp
// Adicionar em v018-valor-10-reais-novo-teclado.ino

class NonBlockingDelay {
  private:
    unsigned long start_time;
    unsigned long duration;
    bool running;
    
  public:
    NonBlockingDelay() : running(false) {}
    
    void start(unsigned long ms) {
      start_time = millis();
      duration = ms;
      running = true;
    }
    
    bool isRunning() {
      if (!running) return false;
      
      if (millis() - start_time >= duration) {
        running = false;
        return false;
      }
      return true;
    }
    
    unsigned long remaining() {
      if (!running) return 0;
      unsigned long elapsed = millis() - start_time;
      return (elapsed < duration) ? (duration - elapsed) : 0;
    }
};

// Uso:
NonBlockingDelay timer;
timer.start(5000);
while(timer.isRunning()) {
  mdb.task();  // Continua processando!
}
```

### Helper: Debug de Variáveis Globais

```cpp
// Adicionar para rastrear mudanças suspeitas

#define DEBUG_GLOBALS

#ifdef DEBUG_GLOBALS
  #define SET_VAR(var, val) do { \
    Serial.print(F(#var " mudou de ")); \
    Serial.print(var); \
    Serial.print(F(" para ")); \
    Serial.println(val); \
    var = val; \
  } while(0)
#else
  #define SET_VAR(var, val) var = val
#endif

// Uso:
SET_VAR(controle_vmc, 1);  // Loga: "controle_vmc mudou de 0 para 1"
```

---

## 📊 Métricas de Sucesso

### Antes da Refatoração
- ❌ 97 variáveis globais
- ❌ 25 `delay()` calls
- ❌ Difícil rastrear bugs
- ❌ Resets escondem problemas

### Após Prioridade 1
- ✅ Delays críticos removidos
- ✅ Código documentado
- ✅ Plano claro de refatoração
- 🟡 97 variáveis globais (ainda)

### Após Prioridade 2
- ✅ Structs definidos
- ✅ Sistema de logging
- ✅ Delays de inicialização corrigidos
- 🟡 97 variáveis globais (preparando migração)

### Após Prioridade 3 (Objetivo Final)
- ✅ ~20 variáveis globais (redução de 80%)
- ✅ 0 delays bloqueantes críticos
- ✅ State machine com enums
- ✅ Telemetria ativa
- ✅ Fácil manutenção e debug

---

## 🎓 Resumo para Quem Vem de Node.js

### Variáveis Globais

```javascript
// Node.js - RUIM (mas menos perigoso por ser single-threaded)
let controle = 0;
let aux = 0;

app.post('/venda', (req, res) => {
  controle = 1;  // Mudou global
});

// Arduino - PIOR (pode causar bugs financeiros)
int controle = 0;
int aux = 0;

void processar_venda() {
  controle = 1;  // Se outra função também mudar...
  // Produto pode sair de graça!
}
```

**Solução:** Encapsular em classes/structs (como em Node.js fazer classes ou módulos)

### Delays Bloqueantes

```javascript
// Node.js - NÃO bloqueia outras requests
app.get('/wait', async (req, res) => {
  await sleep(5000);  // Esta request espera
  // Mas outras requests continuam sendo processadas!
});

// Arduino - BLOQUEIA TUDO
void aguardar() {
  delay(5000);  // TODO o sistema PARA!
  // Nenhuma outra coisa acontece!
}
```

**Solução:** Usar millis() como "polling" manual (como fazer setInterval em Node.js)

---

## 📝 Checklist de Implementação

### Fase 1: Correções Imediatas (Esta Sprint)
- [ ] Substituir delay(5000) em MDB.cpp:151 por espera não-bloqueante
- [ ] Substituir delay(5000) em MDB.cpp:170 por espera não-bloqueante
- [ ] Adicionar comentários explicativos em todas variáveis globais
- [ ] Criar este documento REFACTORING_GUIDE.md ✅
- [ ] Testar que MDB continua funcionando após mudanças

### Fase 2: Melhorias de Código (Próxima Sprint)
- [ ] Definir structs VMCState, MDBState, etc em arquivo .h
- [ ] Substituir delay(100) em MDB.cpp:1005
- [ ] Implementar classe NonBlockingDelay
- [ ] Adicionar sistema de log de erros na EEPROM
- [ ] Testar extensivamente

### Fase 3: Refatoração Profunda (Médio Prazo)
- [ ] Migrar 10 variáveis para VMCState
- [ ] Testar vendas completas
- [ ] Migrar 15 variáveis para MDBState
- [ ] Testar com moedeiro real
- [ ] Continuar migração gradual
- [ ] Implementar enums para state machine
- [ ] Adicionar telemetria

---

**Criado por:** GitHub Copilot  
**Data:** 2025-11-20  
**Status:** 🔴 Aguardando Implementação  
**Próximo Passo:** Executar Fase 1 - Correções Imediatas
