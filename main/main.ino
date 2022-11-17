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
#define recebeFluxo 27
#define escola 32
#define canteiro1 33
#define recebeUmidade 34

#define serialTime 115200

// Variáveis Time
//const char* ntpServer1 = "pool.ntp.org";
//const char* ntpServer2 = "time.nist.gov";
const char* ntpServer1 = "a.st1.ntp.br";
const char* ntpServer2 = "b.st1.ntp.br";
const long  gmtOffset_sec = -10800;
const int   daylightOffset_sec = -10800;
String data = "";

//Variáveis Umidade
String caminhoUmidade = ""; 

//Variáveis Fluxo
double flowRate;    //Este é o valor que pretende-se calcular
volatile int count; //Este número precisa ser setado como volátil para garantir que ele seja atualizado corretamente durante o processo de interrupção
String caminhoFluxo = "";

void setup() {
  
  Serial.begin(serialTime);
  sntp_set_time_sync_notification_cb( timeavailable );
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
  WiFi.begin(ssid, password);

  //Atuadores
  pinMode(motor, OUTPUT);
  pinMode(fazenda, OUTPUT);
  pinMode(escola, OUTPUT);
  pinMode(canteiro1, OUTPUT);

  //Sensores
  pinMode(recebeUmidade, INPUT);
  pinMode(recebeFluxo, INPUT); //Seta o pino de entrada
  attachInterrupt(digitalPinToInterrupt(recebeFluxo), Flow, RISING);  //Configura o pino do Esp(como interruptor) e define para rodar a função "Flow"

}

void loop() {

  // float DHTtemperatura = DHT.temperature; // O VALOR DE UMIDADE MEDIDO 
  // float DHTumidade = DHT.humidity; // O VALOR DE UMIDADE MEDIDO
  if(WiFi.status() != WL_CONNECTED)
  {
   conectarWifi(); 
  }

    //date = obterData();
    //Serial.println(data);

  count = 0;      // Reseta o contador para iniciarmos a contagem em 0 novamente
  interrupts();   //Habilita a interrupção do pino
  delay (1000);   //Espera 1 segundo
  noInterrupts(); //Desabilita o interrupção do pino

   //Cálculos matemáticos
  flowRate = (count * 2.25);        //Conta os pulsos no último segundo e multiplica por 2,25mL, que é a vazão de cada pulso
  flowRate = flowRate * 60;         //Converte segundos em minutos, tornando a unidade de medida mL/min
  flowRate = flowRate / 1000;       //Converte mL em litros, tornando a unidade de medida L/min
   if(count > 0)
   {
     Serial.println(count);
   } else {
    //Serial.println("Não há sinal");
   }
   Serial.println(flowRate);         //Imprime a variável flowRate no Serial
    
  float umidade = analogRead(recebeUmidade);  
  // Firebase.setString(bd, "/escola/canteiro1/umidade", umidade);
  
  // umidade normal sem colocar na terra 4050 [PRECISA DE ÁGUA]
  // umiade colocando na terra ???
  // umidade colocando na agua 2600 maximo que chegou [NÃO PRECISA DE ÁGUA]
  // padrao entre 4050 X 2600

  //Varifica automatico se estiver libera o controle de liberar ou não água
  //Este sistema se ativo, desabilita o controle remoto manual
  //Revisar o código dessa função antes de ativar.
  canteiroAutomatico(false, umidade);

  if (Firebase.get(bd, "/escola/canteiro1/isOpen")) {
    if (bd.dataType() == "string") {
      String RL1 = bd.stringData();
      if (RL1 == "0") {
        // as ações
        //antes de desligar
        Firebase.setString(bd, "/escola/canteiro1/status", "Fechado");
        digitalWrite(canteiro1, HIGH);
      } else if (RL1 == "1") {
        // as ações
        Firebase.setString(bd, "/escola/canteiro1/status", "Aberto");
        digitalWrite(canteiro1, LOW);

        //Escreve a umidade na base de dados apenas se o canteiro estiver Aberto
        caminhoUmidade = "/escola/canteiro1/umidade/"+obterData();
        Firebase.setString(bd, caminhoUmidade, umidade);

        //Escreve o fluxo de agua na base de dados apenas se o canteiro estiver Aberto
        caminhoFluxo = "/escola/canteiro1/fluxo/"+obterData();
        Firebase.setString(bd, caminhoFluxo, flowRate);
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

void canteiroAutomatico(bool ativo, float umidade)
{
  if(!ativo)
  {
    return;
  }
  
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

  //Formatando Mes
  String mes = String(timeinfo.tm_mon+1);
  mes = formatarData(mes);
  //Formatando Dia
  String dia = String(timeinfo.tm_mday);
  dia = formatarData(dia);
  //Formatando Hora
  String hora = String(timeinfo.tm_hour);
  hora = formatarData(hora);
  //Formatando Min
  String minuto = String(timeinfo.tm_min);
  minuto = formatarData(minuto);
  //Formatando Seg
  String segundo = String(timeinfo.tm_sec);
  segundo = formatarData(segundo);

  String dataVal =  String(timeinfo.tm_year + 1900) + "-" + mes + "-" + dia + 
  " " + hora +":"+ minuto +":"+ segundo;
  Serial.println(dataVal);
  return dataVal;
}

String formatarData(String date)
{
  if(date.length()<2)
  {
    return "0"+date;
  }else {
    return date;
  }

}

// Função de retorno de chamada (é chamado quando o tempo se ajusta via NTP)
void timeavailable(struct timeval *t)
{
  Serial.println("Tem ajuste de tempo do NTP!");
}

void Flow()
{
   count++; //Quando essa função é chamada, soma-se 1 a variável "count" 
}
