/****************************************Copyright (c)*************************************************
**                      Power Vending - A. R. M. Venda Automatica Ltda.
**                             http://www.powervending.com.br
**--------------Informação do Arquivo -----------------------------------------------------------------
** Nome do Arquivo:          NonBlockingDelay.h
** Data Ultima Modificação:  20-11-24
** Ultima Versão:            Sim
** Descrição:                Classe para delays não-bloqueantes
**                           Substitui delay() bloqueante por verificação com millis()
**------------------------------------------------------------------------------------------------------
** Criado por:          GitHub Copilot
** Data de Criação:     20-11-24
********************************************************************************************************/

#ifndef NonBlockingDelay_h
#define NonBlockingDelay_h

#include <Arduino.h>

// ============================================================================
// CLASSE NONBLOCKINGDELAY
// Objetivo: Substituir delay() bloqueante por método não-bloqueante
// Uso: Permite que outras tasks continuem executando durante espera
// ============================================================================

class NonBlockingDelay {
  private:
    unsigned long start_time;      // Tempo de início do delay
    unsigned long duration;        // Duração do delay em ms
    bool running;                  // Flag: delay está ativo
    
  public:
    // Construtor
    NonBlockingDelay() : start_time(0), duration(0), running(false) {}
    
    // Inicia um novo delay não-bloqueante
    // Parâmetros:
    //   ms: duração em milissegundos
    void start(unsigned long ms) {
      start_time = millis();
      duration = ms;
      running = true;
    }
    
    // Verifica se o delay ainda está rodando
    // Retorna:
    //   true: ainda em espera
    //   false: delay completou
    bool isRunning() {
      if (!running) return false;
      
      if (millis() - start_time >= duration) {
        running = false;
        return false;
      }
      return true;
    }
    
    // Retorna o tempo restante do delay em ms
    // Retorna:
    //   tempo restante em ms, ou 0 se não está rodando
    unsigned long remaining() {
      if (!running) return 0;
      
      unsigned long elapsed = millis() - start_time;
      if (elapsed >= duration) {
        return 0;
      }
      return duration - elapsed;
    }
    
    // Retorna o tempo decorrido desde o início em ms
    unsigned long elapsed() {
      if (!running) return 0;
      return millis() - start_time;
    }
    
    // Cancela o delay em andamento
    void cancel() {
      running = false;
    }
    
    // Reinicia o delay com a mesma duração
    void restart() {
      if (duration > 0) {
        start(duration);
      }
    }
    
    // Verifica se está rodando (alias para isRunning)
    bool active() {
      return isRunning();
    }
};

// ============================================================================
// EXEMPLOS DE USO
// ============================================================================

/*
// Exemplo 1: Delay simples não-bloqueante
NonBlockingDelay timer;
timer.start(5000);  // 5 segundos

void loop() {
  if (timer.isRunning()) {
    // Ainda aguardando...
    // Mas outras coisas podem acontecer aqui!
    mdb.task();
    sensor.task();
  } else {
    // Delay completou!
    fazer_algo();
  }
}

// Exemplo 2: Aguardar antes de reset
NonBlockingDelay timer_reset;
timer_reset.start(5000);

while(timer_reset.isRunning()) {
  // Mantém sistema responsivo durante espera
  if (Serial1.available()) {
    Serial1.read();
  }
  // Mostra progresso
  if (timer_reset.elapsed() % 1000 == 0) {
    Serial.print(".");
  }
}
// Agora pode resetar

// Exemplo 3: Timeout em leitura MDB
NonBlockingDelay timeout;
timeout.start(100);  // 100ms timeout

int i = 0;
while(i < 33 && timeout.isRunning()) {
  if(Serial1.available()) {
    data[i] = Serial1.read();
    i++;
  }
}

if (i < 33) {
  Serial.println(F("Timeout!"));
}

// Exemplo 4: Múltiplos timers simultâneos
NonBlockingDelay timer_display;
NonBlockingDelay timer_buzzer;
NonBlockingDelay timer_mdb;

timer_display.start(1000);   // Atualiza display a cada 1s
timer_buzzer.start(500);     // Buzzer por 500ms
timer_mdb.start(200);        // Poll MDB a cada 200ms

void loop() {
  if (!timer_display.isRunning()) {
    atualiza_display();
    timer_display.restart();  // Reinicia para próximo ciclo
  }
  
  if (!timer_buzzer.isRunning()) {
    digitalWrite(BUZZER, LOW);
  }
  
  if (!timer_mdb.isRunning()) {
    mdb.poll();
    timer_mdb.restart();
  }
  
  // Outras tasks continuam rodando!
}
*/

#endif
