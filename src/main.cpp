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

WiFiClientSecure espClient;
PubSubClient     client(espClient);

#define TRIG_ENT  4
#define ECHO_ENT  2
#define TRIG_SAI  26
#define ECHO_SAI  25

#define TOTAL_VAGAS        6
#define DISTANCIA_MIN_CM   1.5f
#define DISTANCIA_MAX_CM   9.5f
#define COOLDOWN_MS        2000UL
#define PAUSA_CROSSTALK_US 5000UL
#define MQTT_RETRY_MS      10000UL 

static unsigned long tempoUltimaEntrada  = 0UL;
static unsigned long tempoUltimaSaida    = 0UL;
static unsigned long tempoUltimoMQTT     = 0UL;
static bool estadoAnteriorEnt = false;
static bool estadoAnteriorSai = false;
static int vagasAtuais   = TOTAL_VAGAS;
static int vagasExibidos = -1;

LiquidCrystal_I2C lcd(0x27, 16, 2);

static void setup_wifi(void) {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }
  espClient.setInsecure();
}

static void tentarConectarMQTT(void) {
  if (client.connected()) return;
  unsigned long agora = millis();
  if ((agora - tempoUltimoMQTT) < MQTT_RETRY_MS) return;
  tempoUltimoMQTT = agora;
  char clientId[40];
  snprintf(clientId, sizeof(clientId), "ESP32_Equipe13_%04X", (unsigned)random(0xFFFF));
  if (client.connect(clientId, mqtt_user, mqtt_pass)) {
    char buf[8]; snprintf(buf, sizeof(buf), "%d", vagasAtuais);
    client.publish(topico_vagas, buf);
  }
}

static void atualizarLCD(void) {
  if (vagasAtuais == vagasExibidos) return;
  vagasExibidos = vagasAtuais;
  lcd.setCursor(0, 0); lcd.printf("Vagas: %-9d", vagasAtuais);
  if (client.connected()) {
      char buf[8]; snprintf(buf, sizeof(buf), "%d", vagasAtuais);
      client.publish(topico_vagas, buf);
  }
}

static float lerDistancia(int pinoTrig, int pinoEcho) {
  digitalWrite(pinoTrig, LOW); delayMicroseconds(2);
  digitalWrite(pinoTrig, HIGH); delayMicroseconds(10);
  digitalWrite(pinoTrig, LOW);
  long duracao = pulseIn(pinoEcho, HIGH, 17400UL);
  if (duracao == 0) return -1.0f;
  return (duracao * 0.0343f) / 2.0f;
}

void setup(void) {
  pinMode(TRIG_ENT, OUTPUT); pinMode(ECHO_ENT, INPUT);
  pinMode(TRIG_SAI, OUTPUT); pinMode(ECHO_SAI, INPUT);
  Wire.begin(21, 22); lcd.init(); lcd.backlight();
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  atualizarLCD();
}

void loop(void) {
  tentarConectarMQTT();
  if (client.connected()) client.loop();
  
  unsigned long agora = millis();
  float distEnt = lerDistancia(TRIG_ENT, ECHO_ENT);
  delayMicroseconds(PAUSA_CROSSTALK_US);
  float distSai = lerDistancia(TRIG_SAI, ECHO_SAI);

  bool ent = (distEnt >= DISTANCIA_MIN_CM && distEnt <= DISTANCIA_MAX_CM);
  bool sai = (distSai >= DISTANCIA_MIN_CM && distSai <= DISTANCIA_MAX_CM);

  if (ent && !estadoAnteriorEnt) {
    unsigned long decorrido = agora - tempoUltimaEntrada;
    if (decorrido >= COOLDOWN_MS) {
      if (vagasAtuais > 0) { vagasAtuais--; atualizarLCD(); }
      tempoUltimaEntrada = agora;
    }
  }
  estadoAnteriorEnt = ent;

  if (sai && !estadoAnteriorSai) {
    unsigned long decorrido = agora - tempoUltimaSaida;
    if (decorrido < COOLDOWN_MS) { 
      if (vagasAtuais < TOTAL_VAGAS) { vagasAtuais++; atualizarLCD(); }
      tempoUltimaSaida = agora;
    }
  }
  estadoAnteriorSai = sai;
}
