#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <GyverButton.h>

#include "main.h"

WiFiClient espClient;
PubSubClient client(espClient);
GButton button(BUTTON_PIN);

String mac = WiFi.macAddress();

const String topics[] = {
  "home/id",
  "home/" + mac + "/type"
  "home/" + mac + "/tx",
  "home/" + mac + "/rx"
};


void callBack(char* topic, byte* payload, unsigned int length) { // Функция в которой обрабатываются все присланные команды
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");


  String strTopic = String(topic); // Строка равная топику
  String strPayload = ""; // Создаем пустую строку, которая будет хранить весь payload


  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    strPayload = strPayload + ((char)payload[i]); // присваеваем значение по столбикам, чтобы получить всю строку
  }
  Serial.println();


  // if (strTopic == topics[1]){ // Полив по команде
  //   is_on = strPayload == "on" ? true : false;
  // }


  if (strTopic == topics[2]) { // Если прилетела команда статус,
    if (strPayload == "status") {
      // client.publish(topics[2].c_str(), is_on ? "on" : "off");
    }
  }
}


void connect() {
  client.setServer(mqtt_server, 1883);
  delay(1000);
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(name)) {
      for (int i = 0; i < 3; i++) {
        client.subscribe(topics[i].c_str());
      }
      client.setCallback(callBack);
      Serial.println("connected");
      Serial.println('\n');
    }
    else {
      Serial.println(" try again in 3 seconds");
      delay(3000);
    }
  }
}



void initWifi(){ // Настройка WIFI
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}



void initWifiUpd() {
  ArduinoOTA.setHostname("BUTTON"); // Задаем имя сетевого порта
  Serial.print("OTA Adress:"); Serial.println(ArduinoOTA.getHostname());
  //ArduinoOTA.setPassword((const char *)"0000"); // Задаем пароль доступа для удаленной прошивки
  ArduinoOTA.begin(); // Инициализируем OTA
}



void setup() {
  Serial.begin(9600);
  
  initWifi();
  delay(100);
  connect();
  
  initWifiUpd();
}



void loop() {
  if (!client.connected()) // Если потеряли подключение
    connect(); // Переподключаемся
  
  client.loop();

  ArduinoOTA.handle(); // Всегда готовы к прошивке

  button.tick();

  static unsigned long timer;
  // Отправляем id модуля раз в минуту
  if (millis() - timer > 10*1000) {
    client.publish(topics[0].c_str(), mac.c_str());
    delay(100);
    client.publish(topics[1].c_str(), name);
    timer = millis();
  }

  if (button.isClick())
    client.publish(topics[3].c_str(), "click");
  if (button.isDouble())
    client.publish(topics[3].c_str(), "double");
  if (button.isHolded())
    client.publish(topics[3].c_str(), "holded");
  if (button.isHold())
    client.publish(topics[3].c_str(), "hold");
}