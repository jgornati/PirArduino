#include <Adafruit_CC3000.h>
#include <SPI.h>
#include <cc3000_PubSubClient.h>
#include <string.h>
//#include "utility/debug.h"

#define WiDo_IRQ   7
#define WiDo_VBAT  5
#define WiDo_CS    10
Adafruit_CC3000 WiDo = Adafruit_CC3000(WiDo_CS, WiDo_IRQ, WiDo_VBAT,
                                       SPI_CLOCK_DIVIDER); // you can change this clock speed

#define WLAN_SSID       "Joeeeeeeeeeeeeeeeeeeeeel"           // cannot be longer than 32 characters!
//#define WLAN_SSID       "ReDWiFi"
#define WLAN_PASS       "joel--JOEL"

// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2
#define TIMEOUT_MS  1000

Adafruit_CC3000_Client client;

boolean mqttOK;
unsigned long lastTime;

// We're going to set our broker IP and union it to something we can use
union ArrayToIp {
  byte array[4];
  uint32_t ip;
};

//ArrayToIp server = { 104, 1, 168, 192 }; //Al reves CASA
//ArrayToIp server = { 10, 0, 168, 192 }; //Al reves Kiosco
ArrayToIp server = { 244, 126, 43, 179 };// servidor gornatilabs.com - > 179.43.126.244
cc3000_PubSubClient mqttclient(server.ip, 1883, callback, client, WiDo);

int detect = 0;

void ISR_interrupcion() {
  detect = 1;
}

void setup() {

  Serial.begin(115200);
  configuro_wifi();
  attachInterrupt(1, ISR_interrupcion, RISING); //configuro la interrupcion 1 pata 2 arduino leonardo cuando hay flanco positivo



}

void loop() {

  if (detect == 1) {
    mqttclient.publish("sen1", "25");
    Serial.println("Detecte movimiento...");
    detect = 0;
  }





  mqttclient.loop();


}


void configuro_wifi() {
  Serial.println(F("Hello, CC3000!\n"));

  /* Initialise the module */
  Serial.println(F("\nInitialising the CC3000 ..."));
  if (!WiDo.begin())
  {
    Serial.println(F("Unable to initialise the CC3000! Check your wiring?"));
    while (1);
  }

  Serial.println(F("Connecting Router/AP"));
  if (!WiDo.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while (1);
  }

  Serial.println(F("Router/AP Connected!"));

  /* Wait for DHCP to complete */
  Serial.println(F("Request DHCP"));
  while (!WiDo.checkDHCP())
  {
    delay(200); // ToDo: Insert a DHCP timeout!
  }

  //displayDriverMode();

  //displayMACAddress();

  /* Display the IP address DNS, Gateway, etc. */
  while (!displayConnectionDetails()) {
    delay(1000);
  }

  // connect to the broker
  if (!client.connected()) {
    client = WiDo.connectTCP(server.ip, 1883);
  }

  // did that last thing work? sweet, let's do something
  if (client.connected()) {
    Serial.println(F("Broker connection OK"));

    if (mqttclient.connect("joel", "joel", "asd")) {
      Serial.println(F("Node connection OK"));
      mqttOK = true;
      mqttclient.subscribe("inTopic");

    } else
      Serial.println(F("Autentificacion Incorrecta!"));

  } else {
    Serial.println(F("No se conecto al Broker"));
  }
}

void callback (char* topic, byte* payload, unsigned int length) {

  //  // As long as length isn't too big, we can add a null to the payload:
  //  payload[length] = '\0';
  //  String strPayload = String((char*)payload);
  //
  //  // Allocate the correct amount of memory for the payload copy
  //  byte* p = (byte*)malloc(length);
  //  // Copy the payload to the new buffer
  //  memcpy(p, payload, length);
  //
  //  //if (length == 4 && (strncmp((const char *)payload, "HOLA",4)==0)) { //otra forma de comparar
  //  if (strPayload == "HOLA\0") {
  //    Serial.println("HOLA DETECTED!");
  //  }
  //  // Free the memory
  //  free(p);
}


void publishIntegerValue(char * topic, long value, boolean retained) {
  String str = String(value);
  const char * valueChar = str.c_str();
  publishStringValue(topic, (char *)valueChar, retained);
}

void publishStringValue(char * topic, char * value, boolean retained) {
  unsigned int plength = strlen(value);
  mqttclient.publish(topic, (uint8_t*)value, plength, retained);
}

/**************************************************************************/
/*!
    @brief  Displays the driver mode (tiny of normal), and the buffer
            size if tiny mode is not being used

    @note   The buffer size and driver mode are defined in cc3000_common.h
*/
/**************************************************************************/
//void displayDriverMode(void)
//{
//#ifdef CC3000_TINY_DRIVER
//  Serial.println(F("CC3000 is configure in 'Tiny' mode"));
//#else
//  Serial.print(F("RX Buffer : "));
//  Serial.print(CC3000_RX_BUFFER_SIZE);
//  Serial.println(F(" bytes"));
//  Serial.print(F("TX Buffer : "));
//  Serial.print(CC3000_TX_BUFFER_SIZE);
//  Serial.println(F(" bytes"));
//#endif
//}

/**************************************************************************/
/*!
    @brief  Tries to read the CC3000's internal firmware patch ID
*/
/**************************************************************************/
//uint16_t checkFirmwareVersion(void)
//{
//  uint8_t major, minor;
//  uint16_t version;
//
//#ifndef CC3000_TINY_DRIVER
//  if (!WiDo.getFirmwareVersion(&major, &minor))
//  {
//    Serial.println(F("Unable to retrieve the firmware version!\r\n"));
//    version = 0;
//  }
//  else
//  {
//    Serial.print(F("Firmware V. : "));
//    Serial.print(major); Serial.print(F(".")); Serial.println(minor);
//    version = major; version <<= 8; version |= minor;
//  }
//#endif
//  return version;
//}

/**************************************************************************/
/*!
    @brief  Tries to read the 6-byte MAC address of the CC3000 module
*/
/**************************************************************************/
//void displayMACAddress(void)
//{
//  uint8_t macAddress[6];
//
//  if (!WiDo.getMacAddress(macAddress))
//  {
//    Serial.println(F("Unable to retrieve MAC Address!\r\n"));
//  }
//  else
//  {
//    Serial.print(F("MAC Address : "));
//    WiDo.printHex((byte*)&macAddress, 6);
//  }
//}


/**************************************************************************/
/*!
    @brief  Tries to read the IP address and other connection details
*/
/**************************************************************************/
bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;

  if (!WiDo.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); WiDo.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); WiDo.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); WiDo.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); WiDo.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); WiDo.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}

