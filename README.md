# Estacionamento Inteligente — Projeto Integrador

> Sistema de controle de vagas com ESP32, sensores ultrassônicos HC-SR04, display LCD I2C e telemetria MQTT em tempo real.

## Sobre o Projeto

Projeto desenvolvido pela **Equipe 13** para o módulo de Projeto Integrador do curso de **Ciência da Computação — UNIFEOB**.

O sistema monitora a entrada e saída de veículos em um estacionamento utilizando dois sensores ultrassônicos, exibe a contagem de vagas em um display LCD 16x2 e publica os dados em um broker MQTT (HiveMQ Cloud) para monitoramento remoto.

## Hardware Utilizado

| Componente | Quantidade |
|---|---|
| ESP32 DevKit V1 | 1 |
| Sensor HC-SR04 | 2 |
| Display LCD 16x2 I2C | 1 |
| Protoboard + Jumpers | — |

## Pinagem

| Função | Pino ESP32 |
|---|---|
| TRIG Entrada | GPIO 4 |
| ECHO Entrada | GPIO 2 |
| TRIG Saída | GPIO 26 |
| ECHO Saída | GPIO 25 |
| LCD SDA | GPIO 21 |
| LCD SCL | GPIO 22 |

## Funcionalidades

- Detecção de entrada/saída de veículos com máquina de estados
- Anti-crosstalk entre sensores ultrassônicos
- Sistema de cooldown contra debouncing físico
- Display LCD com atualização inteligente (sem flickering)
- Conexão WiFi e publicação MQTT via TLS (HiveMQ Cloud)
- Reconexão MQTT assíncrona e não-bloqueante

## Evolução do Projeto

O histórico de commits documenta a evolução completa:
1. Estrutura base com definição de pinos
2. Leitura ultrassônica e detecção de entrada
3. Controle de estado para contagem única
4. Integração do display LCD I2C
5. Otimização do LCD contra flickering
6. Sensor de saída com lógica de incremento
7. Isolamento de crosstalk entre sensores
8. Módulo WiFi para conectividade
9. Protocolo MQTT com WiFiClient
10. Migração para WiFiClientSecure + TLS
11. MQTT assíncrono não-bloqueante
12. Sistema de cooldown nos sensores
13. Versão final com UI aprimorada e logs

## Equipe 13

Desenvolvido por alunos de Ciência da Computação — UNIFEOB, 2026.
