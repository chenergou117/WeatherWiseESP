#include "DHT20.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>              //加载的库文件

DHT20 DHT(&Wire);           // 2nd I2C interface

const char* mqttServer = "xxx.xxx.xxx.xxx.xxx"; //matt服务器 IP
const int mqttPort = 1883;//mqtt端口号 默认1883
const char* mqttUser = "ccc"; // 用户
const char* mqttPassword = "ccc";  //密码
const char* mqttTopic = "temp";  //主题

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  Wire.begin(2, 0);    // 选择引脚号；esp8266 01s的话 用2号脚接sda  0号 scl
  if (!DHT.begin()) {
    Serial.println("DHT20 failed to start");
  }

  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");

  client.setServer(mqttServer, mqttPort);
  connectToMqtt();
}

void loop() {
  if (!client.connected()) {
    connectToMqtt();
  }
  client.loop();
  
 // 读取数据
  int status = DHT.read();
  if (status == DHT20_OK) {               // 获取温度和湿度
    float temperature = DHT.getTemperature();
    float humidity = DHT.getHumidity();

    String mac = WiFi.macAddress();
    mac.replace(":", "");   // 去除MAC地址中的冒号
    String payload = "{\"mac\":\"" + mac + "\",\"temperature\":" + String(temperature, 1) + ",\"humidity\":" + String(humidity, 1) + "}";

    client.publish(mqttTopic, payload.c_str());
  }
   // 发送数据
  
  delay(60000); //发送间隔；默认60s
}

void connectToMqtt() {
  String clientId = "ESP8266Client-";
  clientId += WiFi.macAddress();
  clientId.replace(":", "");

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect(clientId.c_str(), mqttUser, mqttPassword)) {
      Serial.println("Connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
