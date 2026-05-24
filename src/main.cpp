#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// 1. CONFIGURACOES DE REDE E NUVEM
const char* ssid         = "stalin";
const char* password     = "palmeiras";
const char* mqtt_server  = "3c36328d74d64bee879a2ce48ec2eff5.s1.eu.hivemq.cloud";
const int   mqtt_port    = 8883;
const char* mqtt_user    = "joao396";
const char* mqtt_pass    = "Turiba14";
const char* topico_vagas = "joao/estacionamento/vagas";

WiFiClientSecure espClient;
PubSubClient     client(espClient);

// 2. PINOS
#define TRIG_ENT  4
#define ECHO_ENT  2
#define TRIG_SAI  26
#define ECHO_SAI  25

// 3. PARAMETROS DO SISTEMA
#define TOTAL_VAGAS        6
#define DISTANCIA_MIN_CM   1.5f
#define DISTANCIA_MAX_CM   9.5f
#define COOLDOWN_MS        2000UL
#define PAUSA_CROSSTALK_US 5000UL
#define MQTT_RETRY_MS      10000UL  

// Variaveis de Controle
static unsigned long tempoUltimaEntrada  = 0UL;
static unsigned long tempoUltimaSaida    = 0UL;
static unsigned long tempoUltimoMQTT     = 0UL;
static bool estadoAnteriorEnt = false;
static bool estadoAnteriorSai = false;

static int vagasAtuais   = TOTAL_VAGAS;
static int vagasExibidos = -1;

LiquidCrystal_I2C lcd(0x27, 16, 2);

// ----------------------------------------------
// FUNCOES BASE
// ----------------------------------------------
static void setup_lcd(void) {
  Wire.begin(21, 22);
  lcd.init();
  delay(100);
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(" Estacionamento");
  lcd.setCursor(0, 1);
  lcd.print(" Iniciando...  ");
}

static void setup_wifi(void) {
  Serial.print("\nConectando ao WiFi: ");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Conectando WiFi");
  WiFi.begin(ssid, password);

  int dots = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcd.setCursor(dots % 16, 1);
    lcd.print(".");
    dots++;
  }

  Serial.println("\nWiFi Conectado!");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Conectado!");
  delay(1000);
  espClient.setInsecure();
}

static void tentarConectarMQTT(void) {
  if (client.connected()) return;

  unsigned long agora = millis();
  if ((agora - tempoUltimoMQTT) < MQTT_RETRY_MS) return;
  tempoUltimoMQTT = agora;

  Serial.print("Tentando conexao MQTT... ");
  char clientId[40];
  snprintf(clientId, sizeof(clientId), "ESP32_Equipe13_%04X", (unsigned)random(0xFFFF));

  if (client.connect(clientId, mqtt_user, mqtt_pass)) {
    Serial.println("MQTT Conectado!");
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", vagasAtuais);
    client.publish(topico_vagas, buf);
  } else {
    Serial.printf("Falhou (rc=%d), tentando em %ds...\n", client.state(), MQTT_RETRY_MS / 1000);
  }
}

static void publicarVagas(void) {
  if (!client.connected()) return;
  char buf[8];
  snprintf(buf, sizeof(buf), "%d", vagasAtuais);
  client.publish(topico_vagas, buf);
}

static void atualizarLCD(void) {
  if (vagasAtuais == vagasExibidos) return;
  vagasExibidos = vagasAtuais;

  char linha0[17];
  char linha1[17];
  snprintf(linha0, sizeof(linha0), "Vagas: %-9d", vagasAtuais);
  snprintf(linha1, sizeof(linha1), "Status: %-8s",
           vagasAtuais <= 0 ? "LOTADO" : "LIVRE");

  lcd.setCursor(0, 0);
  lcd.print(linha0);
  lcd.setCursor(0, 1);
  lcd.print(linha1);
}

// ----------------------------------------------
// LOGICA DE DETECCAO
// ----------------------------------------------
static float lerDistancia(int pinoTrig, int pinoEcho) {
  digitalWrite(pinoTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(pinoTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinoTrig, LOW);

  long duracao = pulseIn(pinoEcho, HIGH, 17400UL);
  if (duracao == 0) return -1.0f;
  return (duracao * 0.0343f) / 2.0f;
}

static void processarSistema(unsigned long agora) {
  float distEnt = lerDistancia(TRIG_ENT, ECHO_ENT);
  delayMicroseconds(PAUSA_CROSSTALK_US);
  float distSai = lerDistancia(TRIG_SAI, ECHO_SAI);

  bool ent = (distEnt >= DISTANCIA_MIN_CM && distEnt <= DISTANCIA_MAX_CM);
  bool sai = (distSai >= DISTANCIA_MIN_CM && distSai <= DISTANCIA_MAX_CM);

  Serial.printf("[DEBUG] distENT=%.2fcm distSAI=%.2fcm | ENT=%d SAI=%d | antENT=%d antSAI=%d | vagas=%d\n",
                distEnt, distSai, ent, sai, estadoAnteriorEnt, estadoAnteriorSai, vagasAtuais);

  if (ent && !estadoAnteriorEnt) {
    unsigned long decorrido = agora - tempoUltimaEntrada;
    if (decorrido >= COOLDOWN_MS) {
      if (vagasAtuais > 0) {
        vagasAtuais--;
        atualizarLCD();
        publicarVagas();
        Serial.println(">>> Carro Entrou!");
      }
      tempoUltimaEntrada = agora;
    } else {
      Serial.printf("[ENT IGNORADA] Cooldown: %lums restantes\n", COOLDOWN_MS - decorrido);
    }
  }
  estadoAnteriorEnt = ent;

  if (sai && !estadoAnteriorSai) {
    unsigned long decorrido = agora - tempoUltimaSaida;
    if (decorrido >= COOLDOWN_MS) {
      if (vagasAtuais < TOTAL_VAGAS) {
        vagasAtuais++;
        atualizarLCD();
        publicarVagas();
        Serial.println(">>> Carro Saiu!");
      }
      tempoUltimaSaida = agora;
    } else {
      Serial.printf("[SAI IGNORADA] Cooldown: %lums restantes\n", COOLDOWN_MS - decorrido);
    }
  }
  estadoAnteriorSai = sai;
}

// ----------------------------------------------
// SETUP E LOOP
// ----------------------------------------------
void setup(void) {
  Serial.begin(115200);
  pinMode(TRIG_ENT, OUTPUT);
  pinMode(ECHO_ENT, INPUT);
  pinMode(TRIG_SAI, OUTPUT);
  pinMode(ECHO_SAI, INPUT);

  setup_lcd();
  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
  atualizarLCD();
}

void loop(void) {
  tentarConectarMQTT();  
  if (client.connected()) client.loop();
  processarSistema(millis());
}
