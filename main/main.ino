// Bibliotecas
#include <SD.h>
#include <FirebaseESP32.h>
#include <WiFi.h>
#include "time.h"
#include "sntp.h"
// #include "sntp.h"
// #include "dht.h" //INCLUSÃO DE BIBLIOTECA Umidade/Temperatura


// Wifi
// #define ssid "Vitor"
// #define password "Bv082930"
// #define ssid "WIFI UNIFEOB"
// #define password ""
#define ssid "LAPTOPIOT"
#define password "senha123"

// FireBase
#define FIREBASE_HOST "https://unifeob-iot-2022-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "dX7Eaolsu8Xx73vNEyKvKuEACfBUuKiOXCL5rDiM"
FirebaseData bd;

// Componentes
#define recebeUmidade 33
#define ledDesligado 14
#define ledLigado 13
#define serialTime 115200

// Variáveis Time
const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long  gmtOffset_sec = -5400;
const int   daylightOffset_sec = -5400;

const char* time_zone = "CET-1CEST,M3.5.0,M10.5.0/3";  // Regra de fuso horário para Europa/Roma, incluindo regras de ajuste de luz do dia (opcional)



void setup() {
  
  Serial.begin(serialTime);
  sntp_set_time_sync_notification_cb( timeavailable );
   configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
  WiFi.begin(ssid, password);
  pinMode(ledDesligado, OUTPUT);
  pinMode(ledLigado, OUTPUT);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    
  }
  
  Serial.println("");
  Serial.println("Wifi Conectado!!!");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  
}

void loop() {

  // float DHTtemperatura = DHT.temperature; // O VALOR DE UMIDADE MEDIDO 
  // float DHTumidade = DHT.humidity; // O VALOR DE UMIDADE MEDIDO
  
  float umidade = analogRead(recebeUmidade);  
  // Serial.print(umidade);
  // Firebase.setString(bd, "/escola/canteiro1/umidade", umidade);
  
  // umidade normal sem colocar na terra 4050 [PRECISA DE ÁGUA]
  // umiade colocando na terra ???
  // umidade colocando na agua 2600 maximo que chegou [NÃO PRECISA DE ÁGUA]
  // padrao entre 4050 X 2600

  //Varifica automatico se estiver libera o controle de liberar ou não água
  if (Firebase.get(bd, "/escola/canteiro1/automatico")) {
    if (bd.dataType() == "string") {
      String automatico = bd.stringData();
      if (automatico == "1") {

        if (umidade < 3600) {

          Firebase.setString(bd, "/escola/canteiro1/sl1", "1");
          // Firebase.pushString(bd, "/canteiro2/historicoUmidade", umidade);// maoir de 4000

        } else {

          // Firebase.pushString(bd, "/canteiro2/historicoUmidade", umidade);//
          Firebase.setString(bd, "/escola/canteiro1/sl1", "0");
        }
        
      }
    }
  }

  if (Firebase.get(bd, "/escola/canteiro1/sl1")) {
    if (bd.dataType() == "string") {
      String RL1 = bd.stringData();
      if (RL1 == "0") {
        // as ações
        // Firebase.pushString(bd, "/escola/canteiro1/ultimoStatusUmidade", umidade);  //antes de desligar
        Firebase.setString(bd, "/escola/canteiro1/statusSl1", "Desligado");

        salvaData();
        digitalWrite(ledDesligado, HIGH);
        digitalWrite(ledLigado, LOW);

      } else if (RL1 == "1") {
        // as ações
        Firebase.setString(bd, "/escola/canteiro1/statusSl1", "Ligado");

        salvaData();
        digitalWrite(ledDesligado, LOW);
        digitalWrite(ledLigado, HIGH);
      }
      
    }
    
  }

}

void salvaData()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Não há tempo disponivel (ainda)");
    return;
  }

  String dataVal = (&timeinfo, "%Y/%m/%d");
  String horaVal = (&timeinfo, "%H:%M:%S");
  Serial.println(dataVal);
}

// Função de retorno de chamada (é chamado quando o tempo se ajusta via NTP)
void timeavailable(struct timeval *t)
{
  Serial.println("Tem ajuste de tempo do NTP!");
}
