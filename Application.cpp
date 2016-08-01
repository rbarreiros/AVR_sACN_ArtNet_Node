
#include "Application.h"

#include <EtherCard.h>
#include <EEPROMex.h>
#include <DMXSerial.h>

#define PORTS 1

// ethernet interface mac address, must be unique on the LAN
//static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
static byte mymac[] = { 0x00,0x23,0x8b,0xdc,0x66,0x40 };
byte Ethernet::buffer[600]; 

#define LED A3

Application::Application()
{
  if(!loadConfig())
    setDefaults();

  pinMode(LED, OUTPUT);
}


void Application::begin(void)
{
  // Starting Ethernet
  if (ether.begin(sizeof(Ethernet::buffer), mymac) == 0)
  {
    // Do some mumbo jumbo to warn user of the error
    // leds certainly
  }

  configureNetwork();

  artnet = new ArtNet(sizeof(Config) + 1, PORTS,
                      (config.iptype == DHCP));
  /*
  artnet = new ArtNet(mymac,
                      sizeof(Config) + 1,
                      Ethernet::buffer + UDP_DATA_P,
                      sizeof(Ethernet::buffer) - UDP_DATA_P,
                      0, // setip
                      0, // artsend
                      0, // callback
                      PORTS);


  artnet->Configure(config.iptype == DHCP, ether.myip, ether.broadcastip);
  */

  // Start DMX
  DMXSerial.init(DMXController);
  DMXSerial.maxChannel(DMXSERIAL_MAX);
}

bool Application::loadConfig(void)
{
  EEPROM.readBlock(0, config);
  return (config.valid == EEPROM_CONFIG);
}

void Application::saveConfig(void)
{
  EEPROM.writeBlock(0, config);
}

void Application::setDefaults(void)
{
  config.iptype = DHCP;
  config.valid = EEPROM_CONFIG;
}

bool Application::configureNetwork(void)
{
  // Configure IP address
  if (config.iptype == DHCP)
  {
    if (!ether.dhcpSetup())
    {
      
      // Warn user through leds ?
      return false;
    }
  }
  else if (config.iptype == CUSTOM)
  {
    ether.staticSetup(config.ip, config.gateway);
  }
  else if (config.iptype == PRIMARY)
  {
    uint8_t ip[] = {2, (uint8_t)(mymac[3]+OEM_HI+OEM_LO), mymac[4], mymac[5]};
    uint8_t gwy[] = {2, 0, 0, 1};
    ether.staticSetup(ip, gwy);
  }
  else if (config.iptype == SECONDARY)
  {
    uint8_t ip[] = {10, (uint8_t)(mymac[3]+OEM_HI+OEM_LO), mymac[4], mymac[5]};
    uint8_t gwy[] = {10, 0, 0, 1};
    ether.staticSetup(ip, gwy);
  }
  else
  {
    // Should never happen !!
    return false;
  }

  // Register listener
    ether.enableBroadcast();
  //ether.udpServerListenOnPort(&artnetPacket, UDP_PORT_ARTNET);
  
  return true;
}

void Application::loop(void)
{
  ether.packetLoop(ether.packetReceive());
}
