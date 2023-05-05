/**
 * A trivial C/MRI -> JMRI interface
 * =================================
 * Sets up pin 13 (LED) as an output, and attaches it to the first output bit
 * of the emulated SMINI interface.
 * 
 * To set up in JMRI:
 * 1: Create a new connection, 
 *    - type = C/MRI, 
 *    - connection = Serial, 
 *    - port = <arduino's port>, 
 *    - speed = 9600
 * 2: Click 'Configure C/MRI nodes' and create a new SMINI node
 * 3: Click 'Add Node' and then 'Done'
 * 4: Restart J/MRI and it should say "Serial: using Serial on COM<x>" - congratulations!
 * 5: Open Tools > Tables > Lights and click 'Add'
 * 6: Add a new light at hardware address 1, then click 'Create' and close the window. Ignore the save message.
 * 7: Click the 'Off' state button to turn the LED on. Congratulations!
 * 
 * Debugging:
 * Open the CMRI > CMRI Monitor window to check what is getting sent.
 * With 'Show raw data' turned on the output looks like:
 *    [41 54 01 00 00 00 00 00]  Transmit ua=0 OB=1 0 0 0 0 0 
 * 
 * 0x41 = 65 = A = address 0
 * 0x54 = 84 = T = transmit, i.e. PC -> C/MRI
 * 0x01 = 0b00000001 = turn on the 1st bit
 * 0x00 = 0b00000000 = all other bits off
 */

#include <CMRI.h>
#include <Ethernet.h>

byte mac[6] = { 0x90, 0xA2, 0xDA, 0xA3, 0xB6, 0x3F };
IPAddress ip(192, 168, 100, 120);
IPAddress gateway(192, 168, 100, 1);
IPAddress subnet(255, 255, 255, 0);

#define TCP_PORT 9006
#define CMRI_NODE_ADDRESS 1

EthernetServer server(TCP_PORT);
EthernetClient jmriClient;

CMRI cmri(CMRI_NODE_ADDRESS, 24, 48, jmriClient);

void setup() {
  Ethernet.init(46);
  Ethernet.begin(mac, ip, gateway, subnet);
  delay(250);
  
  Serial.begin(19200); // just for console debug output, not CMRI comms
  pinMode(13, OUTPUT);

  InitialiseConfig();

  waitForJMRI();
}

void loop() {
  waitForJMRI();
  UpdateSensors();
  // 1: main processing node of cmri library
  cmri.process();
  
  // 2: update output. Reads bit 0 of T packet and sets the LED to this
  digitalWrite(13, cmri.get_bit(0));
}

bool waitForJMRI() {
  bool jmriConnected = jmriClient.connected();
  while (!jmriConnected) {
    jmriClient = server.accept();
    if (jmriClient) {
      jmriConnected = true;
      Serial.println("JMRI Connected");
    }
  }
  return true;
}

void InitialiseConfig() {
  pinMode(2,INPUT_PULLUP);
}

void UpdateSensors() {
  cmri.set_bit(0,digitalRead(2));
}
