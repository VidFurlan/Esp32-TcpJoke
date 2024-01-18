#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <iostream>
#include <TimerEvent.h>
#include "display.hpp"

String dialog[] = {
    String("Hi, I'd like to \nhear a TCP joke."),
    String("Hello, would you \nlike to hear a \nTCP joke?"),
    String("Yes, I'd like to \nhear a TCP joke."),
    String("OK, I'll tell you a TCP joke."),
    String("Ok, I will hear a \nTCP joke."),
    String("Are you ready to \nhear a TCP joke?"),
    String("Yes, I am ready to hear a TCP joke."),
    String("Ok, I am about to \nsend the TCP joke. \nIt will last 10 \nseconds, it has two \ncharacters, and it \nends with a \npunchline."),
    String("Ok, I am ready to \nget your TCP joke \nthat will last 10 \nseconds, has two \ncharacters, and ends with a punchline."),
    String("I'm sorry, your \nconnection has timed \nout. \n> Hello, would you \nlike to hear a TCP \njoke?"),
};

uint8_t receiverAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
esp_now_peer_info_t peerInfo;

typedef struct struct_message
{
  bool master = false;
} struct_message;

struct_message dataToSend[10];
struct_message receivedData;
int step = 0;

bool isMaster = false;
bool hasMaster = false;

const unsigned int timeoutTimerPeriod = 30000;
TimerEvent timeoutTimer;

void timeoutCallback();

static bool masterSlave(int pin);
static void espNowAddPeerAddr();
static void sendData();
static void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
static void onDataReceive(const uint8_t *mac_addr, const uint8_t *incomingData, int len);
static void espNowSetup();
static void createJoke();

void setup()
{
  Serial.begin(115200);  
  Serial.print("\x1b[2J");

  timeoutTimer.set(timeoutTimerPeriod, timeoutCallback);

  initDisplay();
  //terminalTextDisplay("Hello world!", 1, 1, false);

  createJoke();
  espNowSetup();
}

void loop()
{
  if (isMaster)
    timeoutTimer.update();

  if (step >= 9 && isMaster) 
    step = 1;

  if (dataToSend[step].master && isMaster)
  {
    sendData();
    delay(5000);
  }
}

// Timeout callback (if master doesnt hear back from slave)
void timeoutCallback() {
  Serial.println("Timeout called!");
  step -= 1;
  sendData();
}

// See if ESP master or slave
static bool masterSlave(int pin)
{
  pinMode(pin, INPUT);
  // Serial.println(digitalRead(pin));
  if (digitalRead(pin))
  {
    Serial.println("ESP NOW set to master mode \n");
    return true;
  }

  Serial.println("ESP NOW set to slave mode \n");
  return false;
}

static void espNowAddPeerAddr()
{
  // Move the receaver address to the peerInfo var
  for (int i = 0; i < sizeof(receiverAddress); i++)
  {
    peerInfo.peer_addr[i] = receiverAddress[i];
  }
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK && isMaster)
  {
    Serial.println("Failed to add peer(s)");
    return;
  }
}

static void sendData()
{
  //if (master)
    //dataToSend[step].master = true;
  terminalTextDisplay(dialog[step], 1, 1, false);
  esp_err_t result = esp_now_send(receiverAddress, (uint8_t *)&dataToSend[step], sizeof(dataToSend[step]));
  Serial.print("Sending: ");
  Serial.println(dialog[step]);
  Serial.print("On step: ");
  Serial.println(step);
  step += 1;
}

static void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nLast Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Successful" : "Failed");
}

static void onDataReceive(const uint8_t *mac_addr, const uint8_t *incomingData, int len)
{
  if (isMaster) {
    timeoutTimer.reset();
    Serial.println("Canceled timeout timer!");
  }

  step += 1;
  char macStr[18];
  Serial.println("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  Serial.println(receivedData.master ? "Master: " : "Slave: ");
  // Serial.println(dialog[step]);

  if (receivedData.master)
  {
    for (int i = 0; i < sizeof(receiverAddress); i++)
    {
      receiverAddress[i] = mac_addr[i];
    }

    espNowAddPeerAddr();
    sendData();
    if (step >= 9) {step = 2;}
    // Serial.println("sending to master");
  }
  else {

  }
}

static void espNowSetup()
{
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Didn't initializr ESP NOW");
    return;
  }
  Serial.println("ESP NOW setup complete");

  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onDataReceive);

  isMaster = masterSlave(4);
  if (isMaster)
  {
    espNowAddPeerAddr();
  }
}

static void createJoke()
{
  for (int i = 0; i < 5; i += 1)
  {
    dataToSend[i * 2].master = true;
  }
}