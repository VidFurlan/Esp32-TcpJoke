#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <iostream>

uint8_t receiverAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
esp_now_peer_info_t peerInfo;

typedef struct struct_message
{
  bool master = false;
  String message = "";
} struct_message;

struct_message dataToSend[10];
struct_message receivedData;
int step = 0;

bool isMaster = false;
bool hasMaster = false;

/*See if esp is master or slave - if the
desegnated pin has voltage its in master mode */
static bool master_slave(int pin)
{
  pinMode(pin, INPUT);

  if (digitalRead(pin))
  {
    Serial.println("ESP NOW set to master mode \n");
    return true;
  }

  Serial.println("ESP NOW set to slave mode \n");
  return false;
}

static void esp_now_add_peer_addr()
{
  // Move the receaver address to the peerInfo var
  for (int i = 0; i < sizeof(receiverAddress); i++)
  {
    peerInfo.peer_addr[i] = receiverAddress[i];
  }
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer(s)");
    return;
  }
}

static void send_data()
{
  esp_err_t result = esp_now_send(receiverAddress, (uint8_t *)&dataToSend[step], sizeof(dataToSend[step]));
  Serial.print("Sending: ");
  Serial.println(dataToSend[step].message);
  Serial.print("On step: ");
  Serial.println(step);
  step += 1;
}

static void on_data_sent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nLast Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Successful" : "Failed");
}

static void on_data_receive(const uint8_t *mac_addr, const uint8_t *incomingData, int len)
{
  step += 1;
  char macStr[18];
  Serial.println("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  Serial.println(receivedData.master ? "Master: " : "Slave: ");
  Serial.println(receivedData.message);

  if (receivedData.master)
  {
    for (int i = 0; i < sizeof(receiverAddress); i++)
    {
      receiverAddress[i] = mac_addr[i];
    }

    esp_now_add_peer_addr();
    if (step > 9) {step = 3;}
    send_data();
  }
}

static void esp_now_setup()
{
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Didn't initializr ESP NOW");
    return;
  }

  esp_now_register_send_cb(on_data_sent);
  esp_now_register_recv_cb(on_data_receive);

  isMaster = master_slave(4);
  if (isMaster)
  {
    esp_now_add_peer_addr();
  }
}
#define SPK1 "\x1b[31m"
#define SPK2 "\x1b[33m"
#define SPKEND "\x1b[0m"
static void create_joke()
{
  dataToSend[0].message = SPK1 "Hi, I'd like to hear a TCP joke." SPKEND;
  dataToSend[1].message = SPK2 "Hello, would you like to hear a TCP joke?" SPKEND;
  dataToSend[2].message = SPK1 "Yes, I'd like to hear a TCP joke." SPKEND;
  dataToSend[3].message = SPK2 "OK, I'll tell you a TCP joke." SPKEND;
  dataToSend[4].message = SPK1 "Ok, I will hear a TCP joke." SPKEND;
  dataToSend[5].message = SPK2 "Are you ready to hear a TCP joke?" SPKEND;
  dataToSend[6].message = SPK1 "Yes, I am ready to hear a TCP joke." SPKEND;
  dataToSend[7].message = SPK2 "Ok, I am about to send the TCP joke. It will last 10 seconds, it has two characters, it does not have a setting, it ends with a punchline." SPKEND;
  dataToSend[8].message = SPK1 "Ok, I am ready to get your TCP joke that will last 10 seconds, has two characters, does not have an explicit setting, and ends with a punchline." SPKEND;
  dataToSend[9].message = SPK2 "I'm sorry, your connection has timed out. Hello, would you like to hear a TCP joke?" SPKEND;

  for (int i = 0; i < 5; i += 1)
  {
    dataToSend[i * 2].master = true;
  }
}

void setup()
{
  Serial.begin(115200);
  create_joke();
  esp_now_setup();
  Serial.print("\x1b[2J");
}

void loop()
{
  if (dataToSend[step].master && isMaster)
  {
    if (step > 9) {step = 2;}
    send_data();
    delay(5000);
  }
}