#include <Arduino.h>

#define TRIG_ENT  4
#define ECHO_ENT  2
#define TRIG_SAI  26
#define ECHO_SAI  25

#define TOTAL_VAGAS 6
int vagasAtuais = TOTAL_VAGAS;

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_ENT, OUTPUT);
  pinMode(ECHO_ENT, INPUT);
  pinMode(TRIG_SAI, OUTPUT);
  pinMode(ECHO_SAI, INPUT);
  Serial.println("Sistema de Estacionamento Iniciado.");
}

void loop() {
  // Estrutura principal aguardando implementacao
  delay(100);
}
