#include <WiFiClient.h>
#include "esp_camera.h"
#include <WiFi.h>
#include "Arduino.h"
#include <WiFiManager.h>
#include <DHT.h>;

const char* ssid = "Naminis 2.4GHz";
const char* password = "naminukas";
const char *post_url = "http://orai.ml/projektas.php"; // Location where images are POSTED

unsigned long previousMillis = 0;
const long interval = 60000;

bool internet_connected = false;
long current_millis;
unsigned long last_capture_millis = 0;

#define DHTPIN 15     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define PIR 12
#define LED 4

DHT dht(DHTPIN, DHTTYPE); 

WiFiClient client;

String hum;  //Stores humidity value
String temp; //Stores temperature value


// CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22





void setup()
{
  Serial.begin(115200);

  if (init_wifi()) { // Connected to WiFi
    internet_connected = true;
    Serial.println("Internet connected");
  }
  else {
    delay(3000);
    ESP.restart();
  }
    dht.begin();
    pinMode(PIR,INPUT);  // define PIR pin as input
    pinMode(LED,OUTPUT);  // define PIR pin as input

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Nepavyko prisijungti su klaidos kodu: 0x%x", err);
  }

}

bool init_wifi()
{

  WiFiManager wifiManager;
  
  return wifiManager.autoConnect("ESP");
  
}

static void take_send_photo()
{

    
  
  Serial.println("Fotografuojama...");
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;

  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Nepavyko padaryti nuotraukos");
    return;
  }

  
  
  
   if (!client.connect("orai.ml", 80)) 
   {
    Serial.println("Nepavyko prisijungti");   
   }
   String  dataa;
      dataa =  F("POST /projektas.php HTTP/1.1\r\n");
      dataa += F("Content-Type: image/jpg\r\n");
      dataa += F("Accept: */*\r\n");
      dataa += F("Host: orai.ml\r\n");
      dataa += F("Connection: keep-alive\r\n");
      dataa += F("content-Length: ");
      dataa += fb->len;
      dataa += "\r\n";
      dataa += "\r\n";

Serial.println(dataa);
   client.print(dataa);
   client.write((uint8_t *)fb->buf, fb->len);

   delay(20);

   while(client.connected()) 
   {
    if (client.available()) 
    {
      String serverRes = client.readStringUntil('\r');
        Serial.println(serverRes);
    }
   }


  esp_camera_fb_return(fb);

  last_capture_millis = millis();

}

void sendSensorData(){

   hum = dht.readHumidity();
   temp= dht.readTemperature();

  String data = "temp=" + temp + "&hum=" + hum + "&temp1=-127.0&temp2=-127.0";
  
  if(client.connect("orai.ml",80)){
    
    client.println("POST /jutikliai.php HTTP/1.1");
    client.println("Host: orai.ml");
    client.println("Cache-Control: no-cache");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(data.length());
    client.println();
    client.println(data);
  
    Serial.println(data);
    while (client.connected())
    {
      if ( client.available() )
      {
        char str=client.read();
       Serial.print(str);
      }    
     
     }
     Serial.println();
     client.stop();
  }
  else {
    Serial.println("nepavyko issiusti duomenu");
  }
}


void loop()
{



  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    sendSensorData();
  }

   if (currentMillis - last_capture_millis >= 10) {

      if (digitalRead(PIR) == HIGH) {
    take_send_photo();
  }
  }

  yield();

  /*
    esp_sleep_enable_timer_wakeup(5000000); //10 seconds
    int ret = esp_light_sleep_start();
    Serial.printf("light_sleep: %d\n", ret);
    */
}
