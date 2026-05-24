#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

const char* ssid         = "stalin";
const char* password     = "palmeiras";
const char* mqtt_server  = "3c36328d74d64bee879a2ce48ec2eff5.s1.eu.hivemq.cloud";
const int   mqtt_port    = 8883;
const char* mqtt_user    = "joao396";
const char* mqtt_pass    = "Turiba14";
const char* topico_vagas = "joao/estacionamento/vagas";

#define TRIG_ENT  4
#define ECHO_ENT  2
#define TRIG_SAI  26
#define ECHO_SAI  25

#define TOTAL_VAGAS 6
#define PAUSA_CROSSTALK_US 5000UL
#define MQTT_RETRY_MS      10000UL

int vagasAtuais = TOTAL_VAGAS;
int vagasExibidos = -1;
bool estadoAnteriorEnt = false;
bool estadoAnteriorSai = false;
unsigned long tempoUltimoMQTT = 0UL;

WiFiClientSecure espClient;
PubSubClient client(espClient);
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }
  espClient.setInsecure();
}

void tentarConectarMQTT() {
  if (client.connected()) return;
  unsigned long agora = millis();
  if ((agora - tempoUltimoMQTT) < MQTT_RETRY_MS) return;
  tempoUltimoMQTT = agora;
  
  if (client.connect("ESP32_Equipe13", mqtt_user, mqtt_pass)) {
    char buf[8]; snprintf(buf, sizeof(buf), "%d", vagasAtuais);
    client.publish(topico_vagas, buf);
  }
}

void atualizarLCD() {
  if (vagasAtuais == vagasExibidos) return;
  vagasExibidos = vagasAtuais;
  lcd.setCursor(0, 0); lcd.printf("Vagas: %-9d", vagasAtuais);
  if (client.connected()) {
      char buf[8]; snprintf(buf, sizeof(buf), "%d", vagasAtuais);
      client.publish(topico_vagas, buf);
  }
}

float lerDistancia(int pinoTrig, int pinoEcho) {
  digitalWrite(pinoTrig, LOW); delayMicroseconds(2);
  digitalWrite(pinoTrig, HIGH); delayMicroseconds(10);
  digitalWrite(pinoTrig, LOW);
  long duracao = pulseIn(pinoEcho, HIGH, 17400UL);
  if (duracao == 0) return -1.0f;
  return (duracao * 0.0343f) / 2.0f;
}

void setup() {
  pinMode(TRIG_ENT, OUTPUT); pinMode(ECHO_ENT, INPUT);
  pinMode(TRIG_SAI, OUTPUT); pinMode(ECHO_SAI, INPUT);
  Wire.begin(21, 22); lcd.init(); lcd.backlight();
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  atualizarLCD();
}

void loop() {
  tentarConectarMQTT();
  if (client.connected()) client.loop();

  float distEnt = lerDistancia(TRIG_ENT, ECHO_ENT);
  delayMicroseconds(PAUSA_CROSSTALK_US);
  float distSai = lerDistancia(TRIG_SAI, ECHO_SAI);

  bool ent = (distEnt >= 1.5f && distEnt <= 9.5f);
  bool sai = (distSai >= 1.5f && distSai <= 9.5f);

  if (ent && !estadoAnteriorEnt) { if (vagasAtuais > 0) vagasAtuais--; atualizarLCD(); }
  estadoAnteriorEnt = ent;

  if (sai && !estadoAnteriorSai) { if (vagasAtuais < TOTAL_VAGAS) vagasAtuais++; atualizarLCD(); }
  estadoAnteriorSai = sai;
}
