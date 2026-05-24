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
}

float lerDistancia(int pinoTrig, int pinoEcho) {
  digitalWrite(pinoTrig, LOW); delayMicroseconds(2);
  digitalWrite(pinoTrig, HIGH); delayMicroseconds(10);
  digitalWrite(pinoTrig, LOW);
  long duracao = pulseIn(pinoEcho, HIGH, 17400UL);
  if (duracao == 0) return -1.0f;
  return (duracao * 0.0343f) / 2.0f;
}

void loop() {
  float distEnt = lerDistancia(TRIG_ENT, ECHO_ENT);
  
  if (distEnt >= 1.5f && distEnt <= 9.5f) {
      if (vagasAtuais > 0) vagasAtuais--;
      Serial.printf("Carro detectado! Vagas: %d\n", vagasAtuais);
  }
  delay(100);
}
