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
#define ssid "Toninho"
#define password "clone9270droid"

// FireBase
#define FIREBASE_HOST "https://unifeob-iot-2022-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "dX7Eaolsu8Xx73vNEyKvKuEACfBUuKiOXCL5rDiM"
FirebaseData bd;

// Componentes
#define motor 25
#define fazenda 26
#define escola 32
#define canteiro1 33
#define recebeUmidade 34

#define serialTime 115200

// Variáveis Time
const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long  gmtOffset_sec = -10800;
const int   daylightOffset_sec = -10800;

void setup() {
  
  Serial.begin(serialTime);
  sntp_set_time_sync_notification_cb( timeavailable );
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
  WiFi.begin(ssid, password);
  
  pinMode(motor, OUTPUT);
  pinMode(fazenda, OUTPUT);
  pinMode(escola, OUTPUT);
  pinMode(canteiro1, OUTPUT);
  pinMode(recebeUmidade, INPUT);

}

void loop() {

  // float DHTtemperatura = DHT.temperature; // O VALOR DE UMIDADE MEDIDO 
  // float DHTumidade = DHT.humidity; // O VALOR DE UMIDADE MEDIDO
  if(WiFi.status() != WL_CONNECTED)
  {
   conectarWifi(); 
  }

  
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

          Firebase.setString(bd, "/escola/canteiro1/isOpen", "1");
          // Firebase.pushString(bd, "/canteiro2/historicoUmidade", umidade);// maoir de 4000

        } else {

          // Firebase.pushString(bd, "/canteiro2/historicoUmidade", umidade);//
          Firebase.setString(bd, "/escola/canteiro1/isOpen", "0");
        }
        
      }
    }
  }

  if (Firebase.get(bd, "/escola/canteiro1/isOpen")) {
    if (bd.dataType() == "string") {
      String RL1 = bd.stringData();
      if (RL1 == "0") {
        // as ações
        // Firebase.pushString(bd, "/escola/canteiro1/ultimoStatusUmidade", umidade);  //antes de desligar
        Firebase.setString(bd, "/escola/canteiro1/status", "Fechado");
        digitalWrite(canteiro1, HIGH);
      } else if (RL1 == "1") {
        // as ações
        Firebase.setString(bd, "/escola/canteiro1/status", "Aberto");
        digitalWrite(canteiro1, LOW);
      }
      
    }
    
  }

  if (Firebase.get(bd, "/escola/isOpen")) {
    if (bd.dataType() == "string") {
      String RL1 = bd.stringData();
      if (RL1 == "0") {
        // as ações
        // Firebase.pushString(bd, "/escola/canteiro1/ultimoStatusUmidade", umidade);  //antes de desligar
        Firebase.setString(bd, "/escola/status", "Fechado");
        digitalWrite(escola, HIGH);
      } else if (RL1 == "1") {
        // as ações
        Firebase.setString(bd, "/escola/status", "Aberto");
        digitalWrite(escola, LOW);
      }
      
    }
    
  }

  if (Firebase.get(bd, "/fazenda/isOpen")) {
    if (bd.dataType() == "string") {
      String RL1 = bd.stringData();
      if (RL1 == "0") {
        // as ações
        // Firebase.pushString(bd, "/escola/canteiro1/ultimoStatusUmidade", umidade);  //antes de desligar
        Firebase.setString(bd, "/fazenda/status", "Fechado");
        digitalWrite(fazenda, HIGH);
      } else if (RL1 == "1") {
        // as ações
        Firebase.setString(bd, "/fazenda/status", "Aberto");
        digitalWrite(fazenda, LOW);
      }
      
    }
    
  }

  if (Firebase.get(bd, "/motor/isOn")) {
    if (bd.dataType() == "string") {
      String RL1 = bd.stringData();
      if (RL1 == "0") {
        // as ações
        // Firebase.pushString(bd, "/escola/canteiro1/ultimoStatusUmidade", umidade);  //antes de desligar
        Firebase.setString(bd, "/motor/status", "Desligado");
        digitalWrite(motor, HIGH);
      } else if (RL1 == "1") {
        // as ações
        Firebase.setString(bd, "/motor/status", "Ligado");
        digitalWrite(motor, LOW);
      }
      
    }
    
  }
}
void conectarWifi()
{
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    
  }
  
  Serial.println("");
  Serial.println("Wifi Conectado!!!");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}
String obterData()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Não há tempo disponivel (ainda)");
    return "";
  }

//https://cplusplus.com/reference/ctime/tm/
//tm_sec   int    seconds after the minute   0-61*
//tm_min   int    minutes after the hour     0-59
//tm_hour  int    hours since midnight       0-23
//tm_mday  int    day of the month           1-31
//tm_mon   int    months since January       0-11
//tm_year  int    years since 1900
//tm_wday  int    days since Sunday          0-6
//tm_yday  int    days since January 1       0-365
//tm_isdst int    Daylight Saving Time flag   

  String dataVal =  String(timeinfo.tm_mday) + "/" + String(timeinfo.tm_mon+1) + "/" + String(timeinfo.tm_year + 1900) + 
  " " + String(timeinfo.tm_hour) +":"+ String(timeinfo.tm_min) +":"+ String(timeinfo.tm_sec) ;
  Serial.println(dataVal);
  return dataVal;
}

// Função de retorno de chamada (é chamado quando o tempo se ajusta via NTP)
void timeavailable(struct timeval *t)
{
  Serial.println("Tem ajuste de tempo do NTP!");
}
