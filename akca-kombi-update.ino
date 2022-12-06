#include <ArduinoJson.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClientSecure.h>
#include <CertStoreBearSSL.h>
BearSSL::CertStore certStore;
#include <time.h>

#include <SPI.h>
#include <RF24.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>

#include <WebSocketsClient.h>
#include <SocketIOclient.h>


#define SERVER_IP "10.6.0.7"
#define relay  4

RF24 radio(2, 15);

WiFiClient client;
ESP8266WebServer server(80);
WiFiManager wifiManager;
HTTPClient http;

StaticJsonDocument<300> docSet;
StaticJsonDocument<300> docGet;
StaticJsonDocument<300> docInitSet;
StaticJsonDocument<300> docInitGet;

String api_init = "http://api.akcamodul.com/v1/kombi/init";
String api_loop = "http://api.akcamodul.com/v1/kombi/loop";


boolean codeKontrol;

uint8_t *recv;
int plSize;
int n;

int sdebug = 0;

int Set;
int T1TempKon, T2TempKon, T3TempKon;
int T1oku, T2oku, T3oku;
int firstTempSicaklik = 0;
int secondTempSicaklik = 0;
int thirdTempSicaklik = 0;
int firstTempNem = 0;
int secondTempNem = 0;
int thirdTempNem = 0;
int firstTempBat = 0;
int secondTempBat = 0;
int thirdTempBat = 0;
int modulBaglanti, bat, nem, sicaklik, sicaklikKontrol;

int guncelleme = 0;
int espReset = 0;

byte fark1Kontrol, fark2Kontrol, fark3Kontrol  = 0;

int temp1mac0, temp1mac1, temp1mac2;
int temp2mac0, temp2mac1, temp2mac2;
int temp3mac0, temp3mac1, temp3mac2;
String key = "";
int KombiPower ;
boolean selenoidDurum;
boolean resetDurum;
boolean bildirim;
byte kombi;
boolean eepromYaz = 0;

int tempMode;
byte loopMode = 0;
boolean wifiKontrol = false;
byte beklemeSure ;
int baglantiSayisi;


int httpCode ;


String JsonGet;
String InitJsonGet;

int loopZaman = 10;
unsigned long zaman;
unsigned long sicaklikZaman;
unsigned long readZaman;
unsigned long loopModeZaman;
unsigned long sunucuZaman;


int fark1, fark2, fark3;
//////////////////////SocketIo//////////////////////////////

SocketIOclient socketIO;
String gelenVeri = "";


//////////////////// Cihaz Değişkenleri /////////////////////////////

/*
  int temp1mac2 = 0xA6;
  int temp1mac1 = 0x8B;
  int temp1mac0 = 0x53;
*/

byte addr = 50;
byte addr2 = 100;
byte modulVersion = 12;

String h = String(temp1mac2, HEX);
String j = String(temp1mac1, HEX);
String k = String(temp1mac0, HEX);

//String macTemp = "58:2d:34:" + h + ":" + j + ":" + k;

String macTemp = "a4:c1:38:" + h + ":" + j + ":" + k;


int T1temp, T2temp, T3temp;



/////////////////UPDATE VERİLERİ////////////////////////

const String FirmwareVer = {"1"};
#define URL_fw_Version "/ihsanAkcakoyun/AkcaKombiUpdate/main/version.txt"
#define URL_fw_Bin "https://raw.githubusercontent.com/ihsanAkcakoyun/AkcaKombiUpdate/main/akca-kombi-update.ino.bin"
const char* host = "raw.githubusercontent.com";
const int httpsPort = 443;

// DigiCert High Assurance EV Root CA
const char trustRoot[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD
QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB
CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97
nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt
43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P
T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4
gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO
BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR
TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw
DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr
hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg
06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF
PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls
YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk
CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=
-----END CERTIFICATE-----
)EOF";
X509List cert(trustRoot);



extern const unsigned char caCert[] PROGMEM;
extern const unsigned int caCertLen;


void setClock() {
   // Set time via NTP, as required for x.509 validation
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  /*
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
  */
}
  
void FirmwareUpdate()
{  
  WiFiClientSecure client;
  client.setTrustAnchors(&cert);
  if (!client.connect(host, httpsPort)) {
    Serial.println("Connection failed");
    return;
  }
  client.print(String("GET ") + URL_fw_Version + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      //Serial.println("Headers received");
      break;
    }
  }
  String payload = client.readStringUntil('\n');

  payload.trim();
  if(payload.equals(FirmwareVer) )
  {   
     Serial.println("Device already on latest firmware version"); 
  }
  else
  {
    Serial.println("New firmware detected");
    ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW); 
    t_httpUpdate_return ret = ESPhttpUpdate.update(client, URL_fw_Bin);
        
    switch (ret) {
      case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        ESP.reset();
        break;

      case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        ESP.reset();
        break;

      case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
        break;
    } 
  }
 }  
void connect_wifi();
  



void socketIOEvent(socketIOmessageType_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case sIOtype_DISCONNECT:
         //   USE_SERIAL.printf("[IOc] Disconnected!\n");
         Serial.println("Disconnected");

         
            break;
        case sIOtype_CONNECT:
       Serial.printf("[IOc] Connected to url: %s\n", payload);
       Serial.println("Connected");

            // join default namespace (no auto join in Socket.IO V3)
            socketIO.send(sIOtype_CONNECT, "/");
            break;
        case sIOtype_EVENT:
       //  Serial.printf("[IOc] get event: %s\n", payload);
          gelenVeri = (char*)payload;
         Serial.print("Gelen Veri :"); Serial.println(gelenVeri);
         if(gelenVeri.substring(2,5) == "set"){
          Set = gelenVeri.substring(6,9).toInt();
          Serial.print("set :"); Serial.println(Set);
         }
         if(gelenVeri.substring(2,5) == "pow"){
          KombiPower = gelenVeri.substring(6,7).toInt();
          Serial.print("pow :"); Serial.println(KombiPower);
         }
          if(gelenVeri.substring(2,5) == "rst"){
          espReset = gelenVeri.substring(6,7).toInt();
          Serial.print("reset :"); Serial.println(espReset);
          if(espReset == 1){
            ESP.reset();
          }
         }
        // Serial.println("event");
         //Serial.printf(payload);
        //String msg2 = String((char*)payload);
        // Serial.println(msg2);
         //String msg3=hex(payload, length);
            break;
        case sIOtype_ACK:
     //       USE_SERIAL.printf("[IOc] get ack: %u\n", length);
         //   hexdump(payload, length);
          Serial.println("ack");
            break;
        case sIOtype_ERROR:
         //   USE_SERIAL.printf("[IOc] get error: %u\n", length);
          Serial.println("Error");
         //   hexdump(payload, length);
            break;
        case sIOtype_BINARY_EVENT:
       //     USE_SERIAL.printf("[IOc] get binary: %u\n", length);
           // hexdump(payload, length);
            Serial.println("binary");
            break;
        case sIOtype_BINARY_ACK:
      //      USE_SERIAL.printf("[IOc] get binary ack: %u\n", length);
          //  hexdump(payload, length);
           Serial.println("binary ack");
            break;
    }
}


//////////////////// SETUP /////////////////////////////
void setup() {
  EEPROM.begin(512);
  for(int i = 0; i < 14; i++){
   key += char(EEPROM.read(addr + i));
  }


  if(EEPROM.read(addr2) <= 0){
    eepromYaz = 1;
  }
  
  // Initialize Serial port
  Serial.begin(9600);
  Serial.print("key :"); Serial.println(key);
  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);

  connect_wifi();

  
    socketIO.begin("socket2.akcamodul.com", 80, "/socket.io/?EIO=4&token="+ key);
     socketIO.onEvent(socketIOEvent);

  initBLE();

}



const uint8_t channel[3]   = {37, 38, 39}; // BLE advertisement channel number
const uint8_t frequency[3] = { 2, 26, 80}; // real frequency (2400+x MHz)

struct bleAdvPacket { // for nRF24L01 max 32 bytes = 2+6+24
  uint8_t pduType;
  uint8_t payloadSize;  // payload size
  uint8_t mac[6];
  uint8_t payload[24];
};

uint8_t currentChan = 0;
bleAdvPacket buffer;

void initBLE()
{
  radio.begin();
  radio.setAutoAck(false);
  radio.setDataRate(RF24_1MBPS);
  radio.disableCRC();
  radio.setChannel( frequency[currentChan] );
  radio.setRetries(0, 0);
  radio.setPALevel(RF24_PA_MAX);
  radio.setAddressWidth(4);
  radio.openReadingPipe(0, 0x6B7D9171); // advertisement address: 0x8E89BED6 (bit-reversed -> 0x6B7D9171)
  radio.openWritingPipe(  0x6B7D9171);
  radio.powerUp();
}

void hopChannel()
{
  currentChan++;
  if (currentChan >= sizeof(channel)) currentChan = 0;
  radio.setChannel( frequency[currentChan] );
}

bool receiveBLE(int timeout)
{
  radio.startListening();
  delay(timeout);
  if (!radio.available()) return false;
  while (radio.available()) {
    radio.read( &buffer, sizeof(buffer) );
    swapbuf( sizeof(buffer) );
    whiten( sizeof(buffer) );
  }
  return true;
}

// change buffer content to "wire bit order"
void swapbuf(uint8_t len)
{
  uint8_t* buf = (uint8_t*)&buffer;
  while (len--) {
    uint8_t a = *buf;
    uint8_t v = 0;
    if (a & 0x80) v |= 0x01;
    if (a & 0x40) v |= 0x02;
    if (a & 0x20) v |= 0x04;
    if (a & 0x10) v |= 0x08;
    if (a & 0x08) v |= 0x10;
    if (a & 0x04) v |= 0x20;
    if (a & 0x02) v |= 0x40;
    if (a & 0x01) v |= 0x80;
    *(buf++) = v;
  }
}

void whiten(uint8_t len)
{
  uint8_t* buf = (uint8_t*)&buffer;
  // initialize LFSR with current channel, set bit 6
  uint8_t lfsr = channel[currentChan] | 0x40;
  while (len--) {
    uint8_t res = 0;
    // LFSR in "wire bit order"
    for (uint8_t i = 1; i; i <<= 1) {
      if (lfsr & 0x01) {
        lfsr ^= 0x88;
        res |= i;
      }
      lfsr >>= 1;
    }
    *(buf++) ^= res;
  }
}


char buf[100];
int x, cnt = 0, mode = 0, v1, v10;
int cntOld = -1;
unsigned long tmT = 0;
unsigned long tmH = 0;
unsigned long tmB = 0;
unsigned long tmD = 0;
char *modeTxt = "";
bool isTempOK(int v) {
  return (v >= -400 && v <= 800);
}
bool isHumOK(int v) {
  return (v >= 0 && v <= 1000);
}

//////////////////// LOOP /////////////////////////////
void loop() {
socketIO.loop();

  if (millis() - readZaman > 5000)
  {
    receiveBLE(100);
    recv = buffer.payload;
    plSize = buffer.payloadSize - 6;
    n = plSize <= 24 ? plSize : 24;

    readZaman = millis();
    /*

        Serial.println("**********************************");
        Serial.print("fark 1: "); Serial.println(fark1);
        Serial.print("fark 2: "); Serial.println(fark2);
        Serial.print("fark 3: "); Serial.println(fark3);
        Serial.println("**********************************");
        if (!codeKontrol) {

          Serial.println("**********************************");
          Serial.print("firstTempSicaklik: "); Serial.println(firstTempSicaklik);

          Serial.println("**********************************");
          Serial.print("secondTempSicaklik: "); Serial.println(secondTempSicaklik);

          Serial.println("**********************************");
          Serial.print("thirdTempSicaklik: "); Serial.println(thirdTempSicaklik);
             Serial.println(".-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.");
              Serial.print("ortalama: "); Serial.println(sicaklik);
              Serial.print("setSicaklik :"); Serial.println(Set);
              Serial.print("KombiPower :"); Serial.println(KombiPower);
              Serial.print("Loop Mode :"); Serial.println(loopMode);
              Serial.print("Temp Mode :"); Serial.println(tempMode);
              Serial.println(".-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.");
  */
  
             

  
    hopChannel();
  }

  



  switch (tempMode)
  {
    case 1:
      bluetooth();
      break;

    case 2:
      bluetooth();
      bluetooth2();
    
      break;

    case 3:
      bluetooth();
      bluetooth2();
      bluetooth3();
      break;
  }



  veriler();


  if (loopMode == 0) {
    if (millis() - zaman > (loopZaman * 1000)) {
  //    Serial.print("loopZaman: "); Serial.println(loopZaman);
      jsonWrite();
      jsonRead();
      zaman = millis();
      /*
            Serial.println("*********************************************************");
            Serial.print("temp :"); Serial.println(sicaklik);
            Serial.print("bat :"); Serial.println(bat);
            Serial.print("nem :"); Serial.println(nem);
            Serial.println("*********************************************************");
      */
    }
  }

if(loopMode == 2){
  if( millis() - sunucuZaman > 1800000){
    Serial.print("millis :"); Serial.println(millis());
    ESP.reset();
  }
}

  
  kombiloop();

  if (kombi == 1)
  {
    digitalWrite(relay, HIGH);
   

  }
  if ( kombi == 0)
  {
    digitalWrite(relay, LOW);
    
  }

  wifikontrol();
}




void bluetooth() {

  if (buffer.mac[5] == 0xa4 && buffer.mac[2] == temp1mac2 && buffer.mac[1] == temp1mac1 && buffer.mac[0] == temp1mac0) // limit to my Xiaomi MAC address (1st and last number only)
  {
    if (recv[0] == 16 &&  plSize == 17 && recv[11] != 0 && recv[10] < 2 ) {

      if (firstTempSicaklik < 50 && 0 < (recv[11] + (recv[10] * 255)) && (recv[11] + (recv[10] * 255)) < 450) {

        firstTempSicaklik = recv[11] + (recv[10] * 255);
        T1oku = T1oku + 1;
        Serial.print("Temp 1 Okunma Sayısı: "); Serial.println(T1oku);
      }

      if (100 < (recv[11] + (recv[10] * 255)) && (recv[11] + (recv[10] * 255)) < 450) {
        fark1 = (recv[11] + (recv[10] * 255)) - firstTempSicaklik;

        if (abs(fark1) < 15) {

          firstTempSicaklik = recv[11] + (recv[10] * 255);
          T1TempKon = firstTempSicaklik;
           fark1Kontrol = 0;
         fark1 = 0;
        }
        if(abs(fark1) > 14) {
          firstTempSicaklik =  T1TempKon;
            fark1Kontrol++;
          if(fark1Kontrol > 20)
          {
              firstTempSicaklik = recv[11] + (recv[10] * 255);
              T1TempKon = firstTempSicaklik;
              fark1Kontrol = 0;
              
        
          }
        }
 fark1 = 0;
      }

      if ( firstTempBat == 0 && 0 < recv[13] && recv[13] < 101 )
        firstTempBat = recv[13];
      if (0 < recv[13] && recv[13] < 101) {
        if (-2 < recv[13] - firstTempBat && recv[13] - firstTempBat < 2) {
          firstTempBat = recv[13];
        }
      }

      if (firstTempNem == 0 && 0 < recv[12] && recv[12] < 101 )
        firstTempNem = recv[12] ;
      if (0 < recv[12] && recv[12] < 101 ) {
        if (-2 < recv[12] - firstTempNem && recv[12] - firstTempNem < 2)
        {
          firstTempNem = recv[12] ;
        }
      }
    }
  }
}


void bluetooth2() {
  if (temp2mac0 != 0) {   

  if (buffer.mac[5] == 0xa4 && buffer.mac[2] == temp2mac2 && buffer.mac[1] == temp2mac1 && buffer.mac[0] == temp2mac0) // limit to my Xiaomi MAC address (1st and last number only)
  {
    if (recv[0] == 16 &&  plSize == 17 && recv[11] != 0 && recv[10] < 2 ) {

      if (secondTempSicaklik < 50 && 0 < (recv[11] + (recv[10] * 255)) && (recv[11] + (recv[10] * 255)) < 450) {

        secondTempSicaklik = recv[11] + (recv[10] * 255);
        T2oku++;
        
        Serial.print("Temp 2 Okunma Sayısı: "); Serial.println(T2oku);
      }



      if (100 < (recv[11] + (recv[10] * 255)) && (recv[11] + (recv[10] * 255)) < 450) {
        fark2 = (recv[11] + (recv[10] * 255)) - secondTempSicaklik;
        
        if (abs(fark2) < 15) {

          secondTempSicaklik = recv[11] + (recv[10] * 255);
          T2TempKon = secondTempSicaklik;
          fark2Kontrol = 0;
          fark2 = 0;
        }
        if(abs(fark2 > 14)) {
          secondTempSicaklik =  T2TempKon;
          fark2Kontrol++;
          if (fark2Kontrol > 20)
          {
            secondTempSicaklik = recv[11] + (recv[10] * 255);
            T2TempKon = secondTempSicaklik;
            fark2Kontrol = 0;
          }

        }
        fark2 = 0;

      }

      if ( secondTempBat == 0 && 0 < recv[13] && recv[13] < 101 )
        secondTempBat = recv[13];
      if (0 < recv[13] && recv[13] < 101) {
        if (-2 < recv[13] - secondTempBat && recv[13] - secondTempBat < 2) {
          secondTempBat = recv[13];
        }
      }

      if (secondTempNem == 0 && 0 < recv[12] && recv[12] < 101 )
        secondTempNem = recv[12] ;
      if (0 < recv[12] && recv[12] < 101 ) {
        if (-2 < recv[12] - secondTempNem && recv[12] - secondTempNem < 2)
        {
          secondTempNem = recv[12] ;
        }
      }
    }
  }
 }
}


void bluetooth3() {
  if (temp3mac0 != 0) {

    if (buffer.mac[5] == 0xa4 && buffer.mac[2] == temp3mac2 && buffer.mac[1] == temp3mac1 && buffer.mac[0] == temp3mac0) // limit to my Xiaomi MAC address (1st and last number only)
  {
    if (recv[0] == 16 &&  plSize == 17 && recv[11] != 0 && recv[10] < 2 ) {

      if (thirdTempSicaklik < 50 && 0 < (recv[11] + (recv[10] * 255)) && (recv[11] + (recv[10] * 255)) < 450) {

        thirdTempSicaklik = recv[11] + (recv[10] * 255);
      T3oku++;
        Serial.print("Temp 3 Okunma Sayısı: "); Serial.println(T3oku);
      }



      if (100 < (recv[11] + (recv[10] * 255)) && (recv[11] + (recv[10] * 255)) < 450) {
        fark3 = (recv[11] + (recv[10] * 255)) - thirdTempSicaklik;
        
        if (abs(fark3) < 15) {

          thirdTempSicaklik = recv[11] + (recv[10] * 255);
          T3TempKon = thirdTempSicaklik;
          fark3Kontrol = 0;
          fark3 = 0;
        }
        if(abs(fark3 > 14)) {
          thirdTempSicaklik =  T3TempKon;
          fark3Kontrol++;
          if (fark3Kontrol > 20)
          {
            thirdTempSicaklik = recv[11] + (recv[10] * 255);
            T3TempKon = thirdTempSicaklik;
            fark3Kontrol = 0;
            
          }

        }
        fark3 = 0;

      }

      if ( thirdTempBat == 0 && 0 < recv[13] && recv[13] < 101 )
        thirdTempBat = recv[13];
      if (0 < recv[13] && recv[13] < 101) {
        if (-2 < recv[13] - thirdTempBat && recv[13] - thirdTempBat < 2) {
          thirdTempBat = recv[13];
        }
      }

      if (thirdTempNem == 0 && 0 < recv[12] && recv[12] < 101 )
        thirdTempNem = recv[12] ;
      if (0 < recv[12] && recv[12] < 101 ) {
        if (-2 < recv[12] - thirdTempNem && recv[12] - thirdTempNem < 2)
        {
          thirdTempNem = recv[12] ;
        }
      }
    }
  }
 }
}


void jsonInit() {

  docInitSet["key"] = key;
  docInitSet["bildirim"] = 0;
  docInitSet["modulVersion"] = modulVersion;



  String outputInit = "";
  serializeJsonPretty(docInitSet, outputInit);

  http.begin(client, api_init ); //HTTP
  http.addHeader("Content-Type", "application/json");

  httpCode = http.POST(outputInit);

Serial.print("httpCode :"); Serial.println(httpCode);
  if (httpCode > 0) {
    //Get the request response payload
    InitJsonGet = http.getString();
    //Print the response payload
    Serial.print("InitJsonGet: ");
    Serial.println(InitJsonGet);

    DeserializationError error = deserializeJson(docInitGet, InitJsonGet);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }


    // sunucudan gelen verileri ayrıştırma

    tempMode = docInitGet["tempMode"];
    KombiPower = docInitGet["kombiPower"];
    Set = docInitGet["setSicaklik"];
    String G_mac1 = docInitGet ["Temp1Mac"];
    String G_mac2 = docInitGet ["Temp2Mac"];
    String G_mac3 = docInitGet ["Temp3Mac"];
    String G_status = docInitGet ["status"];
    String G_msg = docInitGet ["msg"];

    

      Serial.print("G_mac1: "); Serial.println(G_mac1);
      temp1mac0 = (G_mac1.substring(8, 12)).toInt();
      temp1mac1 = (G_mac1.substring(4, 7)).toInt();
      temp1mac2 = (G_mac1.substring(0, 3)).toInt();
      
    if(temp1mac0 > 0 && temp1mac1 > 0 && temp1mac2 > 0 && eepromYaz == 1){       
  EEPROM.write(addr2 ,     temp1mac0);
  EEPROM.write(addr2 + 2 , temp1mac1);
  EEPROM.write(addr2 + 4 , temp1mac2);
  EEPROM.commit();
  eepromYaz = 0;
  
  Serial.println("EEPROM'a mac1 Yazıldı..."); 
    }
    
    
    if (tempMode > 1) {

      temp2mac0 = (G_mac2.substring(8, 12)).toInt();
      temp2mac1 = (G_mac2.substring(4, 7)).toInt();
      temp2mac2 = (G_mac2.substring(0, 3)).toInt();

      Serial.print("temp2mac0: "); Serial.println(temp2mac0);
      Serial.print("temp2mac1: "); Serial.println(temp2mac1);
      Serial.print("temp2mac2: "); Serial.println(temp2mac2);
    }
    if (tempMode > 2 ) {

      temp3mac0 = (G_mac3.substring(8, 12)).toInt();
      temp3mac1 = (G_mac3.substring(4, 7)).toInt();
      temp3mac2 = (G_mac3.substring(0, 3)).toInt();

      Serial.print("temp3mac0: "); Serial.println(temp3mac0);
      Serial.print("temp3mac1: "); Serial.println(temp3mac1);
      Serial.print("temp3mac2: "); Serial.println(temp3mac2);
    }

  }
  //Close connection
  http.end();
  
if(httpCode < 1){
  
     Set = 220;
      thirdTempSicaklik = 0;
      secondTempSicaklik = 0;
      KombiPower = 1;
      loopMode = 2;
      tempMode = 1;
    temp1mac0 = EEPROM.read(addr2);
    temp1mac1 = EEPROM.read(addr2 + 2);
    temp1mac2 = EEPROM.read(addr2 + 4);
      Serial.println("Sunucu Bulunamadı...");
      
      Serial.print("temp1mac0: "); Serial.println(temp1mac0);
      Serial.print("temp1mac1: "); Serial.println(temp1mac1);
      Serial.print("temp1mac2: "); Serial.println(temp1mac2);

}


  // Add an array.
  /*
    JsonArray data = doc.createNestedArray("data");
    data.add(48.756080);
    data.add(2.302038);
  */
}

void jsonWrite() {

  docSet["key"] = key;
  docSet["sicaklik"] = sicaklik;
  docSet["nem"] = nem;
  docSet["bat"] = bat;
  docSet["kombi"] = kombi;


  docSet["T1temp"] = firstTempSicaklik;
  docSet["T1Nem"] = firstTempNem;
  docSet["T1Bat"] = firstTempBat;

  docSet["T2temp"] = secondTempSicaklik;
  docSet["T2Nem"] = secondTempNem;
  docSet["T2Bat"] = secondTempBat;

  docSet["T3temp"] = thirdTempSicaklik;
  docSet["T3Nem"] = thirdTempNem;
  docSet["T3Bat"] = thirdTempBat;

  /*
    Serial.print("secondTempSicaklik :"); Serial.println(secondTempSicaklik);
    Serial.print("secondTempBat : %"); Serial.println(secondTempBat);
    Serial.print("secondTempNem : %"); Serial.println(secondTempNem);
    Serial.println(".-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.");

    Serial.print("thirdTempSicaklik :"); Serial.println(thirdTempSicaklik);
    Serial.print("thirdTempBat : %"); Serial.println(thirdTempBat);
    Serial.print("thirdTempNem : %"); Serial.println(thirdTempNem);
    Serial.println(".-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.");
  */

  String output = "";
  //  Serial.println("*************************************");
  serializeJsonPretty(docSet, output);
  // Serial.println(output);
  // Serial.println("*************************************");


  http.begin(client, api_loop); //HTTP
  http.addHeader("Content-Type", "application/json");



  httpCode = http.POST(output);
  // Serial.print("httpCode: "); Serial.println(httpCode);

 sunucuZaman = millis();

}


void jsonRead () {
  


  if (httpCode > 0) {
    //Get the request response payload
    JsonGet = http.getString();
    //Print the response payload
  //  Serial.println("JsonGet: ");
 //   Serial.println(JsonGet);

    DeserializationError error = deserializeJson(docGet, JsonGet);


    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    // sunucudan gelen verileri ayrıştırma
    KombiPower = docGet["kombiPower"];
    Set = docGet["setSicaklik"];
    loopZaman = docGet["loop_sure"];
    byte gunesPower = docGet ["gunesPower"];
    int gunesEnergi = docGet["gunesEnergi"];
    guncelleme = docGet["update"];
   // Serial.print("guncelleme: "); Serial.println(guncelleme);
    espReset = docGet["rst"];
   // Serial.print("espReset: "); Serial.println(espReset);
    /*
        Serial.print("KombiPower: ");    Serial.println(KombiPower);
        Serial.print("Set: ");    Serial.println(Set);
        Serial.print("gunesPower: ");    Serial.println(gunesPower);
        Serial.print("gunesEnergi: ");    Serial.println(gunesEnergi);
    */


  }
  //Close connection
  http.end();

  if(guncelleme == 1){
    setClock();
   FirmwareUpdate();
  }
  if(espReset == 1){
    ESP.reset();
  }

}

void veriler() {
  switch (tempMode)
  {
    case 1:
      sicaklik = firstTempSicaklik;
      nem = firstTempNem;
      bat = firstTempBat;   
      break;

    case 2:
      if (firstTempSicaklik > 0 && secondTempSicaklik > 0 )
        sicaklik = (firstTempSicaklik + secondTempSicaklik) / 2;

      if (firstTempNem > 0 && secondTempNem > 0)
        nem =  (firstTempNem + secondTempNem) / 2;

      if (firstTempBat > 0 && secondTempBat > 0)
        bat =  (firstTempBat + secondTempBat) / 2;
      break;

    case 3:
      if (firstTempSicaklik > 0 && secondTempSicaklik > 0 &&  thirdTempSicaklik > 0)
        sicaklik = (firstTempSicaklik + secondTempSicaklik + thirdTempSicaklik) / 3;

      if (firstTempNem > 0 && secondTempNem > 0 && thirdTempNem > 0)
        nem =  (firstTempNem + secondTempNem + thirdTempNem) / 3;

      if (firstTempBat > 0 && secondTempBat > 0 && thirdTempBat > 0)
        bat =  (firstTempBat + secondTempBat + thirdTempBat) / 3;      
      break;
  }
  
  if(sicaklik < 100){
      if (millis() - sicaklikZaman > 360000){
          ESP.reset();
      }    
  }
  if(sicaklik > 99) {
   sicaklikZaman = millis();
  }
}

void kombiloop() {


  if (sicaklikKontrol == 0) {
    sicaklikKontrol = sicaklik;
  }

  if (KombiPower == 1 && sicaklik > 0)

    if (100 < sicaklik && sicaklik <= 400 )
    {

      if (sicaklik >= Set) //Kombinin çalışmayı durduracağı sıcaklık değeri
      {
        kombi = 0;
        codeKontrol = false;
      }
      if (sicaklik <= (Set - 5)) //Kombinin çalışmaya başlayacağı sıcaklık değeri
      {
        kombi = 1;
        codeKontrol = true;
      }

    }

  if (KombiPower == 0)
  {
    kombi = 0;
  }
}


void  wifikontrol() {
  if (millis() - loopModeZaman > 30000) {

    if (((WiFi.status() != WL_CONNECTED) || (WiFi.localIP().toString() == "0.0.0.0"))) {


      WiFi.reconnect();
    //  wifiManager.setConfigPortalTimeout(10); // Access Point not long because just reconnection
     // wifiManager.setTimeout(10); // Access Point not long because just reconnection
      //wifiManager.autoConnect("AKCA KOMBI");
      Set = 225;
      thirdTempSicaklik = 0;
      secondTempSicaklik = 0;
      KombiPower = 1;
      loopMode = 1;
      tempMode = 1;
      baglantiSayisi++;
      Serial.print("baglantı deneme sayısı :"); Serial.println(baglantiSayisi);
      Serial.print("Set :"); Serial.println(Set);
      Serial.print("Sicaklik: "); Serial.println(sicaklik);
    }
    else {
      if ( loopMode == 1) {
        Serial.println("...WİFİ BULUNDU ESP RESETLENDİ...");
        ESP.reset();
      }
    }

    loopModeZaman = millis();
  }

}


void connect_wifi(){
  wifiManager.setTimeout(180);
  wifiManager.setConfigPortalTimeout(120);

   //wifiManager.resetSettings(); //Wifi ağını resetler

  if (!wifiManager.autoConnect("AKCA KOMBİ")) {
    Serial.println("Secilen Ağa Bağlanamadı. Ayarlar Otomatik Atandı...");
    delay(3000);
    // ESP.reset(); //reset and try again
    temp1mac0 = EEPROM.read(addr2);
    temp1mac1 = EEPROM.read(addr2 + 2);
    temp1mac2 = EEPROM.read(addr2 + 4);
    Set = 225;
    KombiPower = 1;
    wifiKontrol = true;
    tempMode = 1;
    
  }

  if (!wifiKontrol) {
    Serial.println("WiFi connected..");
    server.begin(); Serial.println("Webserver started..."); // Start the webserver

    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print(".");
      delay(300);
    }

    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.print(WiFi.localIP()); Serial.println("/");


    while (!Serial) continue;   
        loopMode = 0;
    jsonInit();  
    

  }
  else {
    loopMode = 1;
  }
  
}
