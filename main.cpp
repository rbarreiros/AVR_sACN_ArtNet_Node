#include <Arduino.h>

#include <EEPROM.h>
#include <EtherCard.h>
#include <ArtNet.h>
#include <DMXSerial.h>

#define PORTS 1

// ethernet interface mac address, must be unique on the LAN
//static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
static byte mymac[] = { 0x00,0x23,0x8b,0xdc,0x66,0x40 };
byte Ethernet::buffer[600]; 

struct Config {
  IPConfiguration iptype;
  byte ip[4];
  byte gateway[4];
  byte valid;
} config;

static void setIP(IPConfiguration iptype, const char *ip, const char *subnet);
static void callback(unsigned short port, const char *buffer, unsigned short length);
static void artSend(size_t length, word sport, byte *dip, word dport);
 
ArtNet artnet(mymac,
              sizeof(config) + 1,
              Ethernet::buffer + UDP_DATA_P,
              sizeof(Ethernet::buffer) - UDP_DATA_P,
              setIP,
              artSend,
              callback,
              PORTS);

// Reset Function
void(* resetFunc) (void) = 0;

static void saveConfig() {
  for (uint16_t i = 0; i < sizeof(config); ++i)
    EEPROM.write(i, ((byte*)&config)[i]);
}

static void loadConfig(void)
{
  for (uint16_t i = 0; i < sizeof(config); ++i)
  {
    ((byte*)&config)[i] = EEPROM.read(i);
  }
  
  if (config.valid != 252)
  {
    // Load defaults
    config.iptype = DHCP;
    config.valid = 252;
    
    saveConfig();
  }
}

static void callback(unsigned short port, const char *buffer, unsigned short length)
{
  if(length > DMXSERIAL_MAX) return;
  
  uint8_t *dmxBuffer = DMXSerial.getBuffer();
  memcpy(&dmxBuffer[1], buffer, length);
}

static void setIP(IPConfiguration iptype, const char *ip, const char *subnet)
{
  config.iptype = iptype;
  memcpy(config.ip, ip, 4);
  // Subnet too??
  // What about the gateway - not really important for ArtNet!
  saveConfig();
  // Restart the chip to load the new configuration
  resetFunc();
}

static void artSend(size_t length, word sport, byte *dip, word dport)
{
  ether.sendUdp((char*)Ethernet::buffer + UDP_DATA_P, length, sport, dip, dport);
}

static void artnetPacket(unsigned int port, unsigned char ip[4], unsigned int what, const char *data, unsigned int len)
{
  artnet.ProcessPacket(ip, port, data, len);
}

const char statusPage[] PROGMEM =
    "HTTP/1.0 200 Ok\r\n"
    "Content-Type: text/html\r\n"
    "Pragma: no-cache\r\n"
    "\r\n"
    "<html>"
      "<head><title>"
        "ArtNet Node Configuration"
      "</title></head>"
      "<body>"
        "<h1>ArtNet Node Configuration</h1>"
        "<h2>Packets: %d (%d failed)</h2>"
        "<p><a href='/ip'>IP Configuration</a></p>"
        "<p><a href='/artnet'>ArtNet Configuration</a></p>"
      "</body>"
    "</html>";

const char ipPage[] PROGMEM =
    "HTTP/1.0 200 Ok\r\n"
    "Content-Type: text/html\r\n"
    "Pragma: no-cache\r\n"
    "\r\n"
    "<html>"
      "<head><title>"
        "ArtNet Node"
      "</title></head>"
      "<body>"
        "<h1>IP Configuration</h1>"
        "<a href='/'>Back</a><p/>"
        "<form action='/ip'>"
          "IP Type:<select name='iptype'>"
            "<option value='2'%s>DHCP</option>"
            "<option value='0'%s>Pri</option>"
            "<option value='1'%s>Sec</option>"
            "<option value='3'%s>Custom</option>"
          "</select><p/>"
          "IP:<input type='text' name='ip' value='%d.%d.%d.%d'><p/>"
          "<input type='submit'>"
        "</form>"
      "</body>"
    "</html>";

const char artnetPage[] PROGMEM =
    "HTTP/1.0 200 Ok\r\n"
    "Content-Type: text/html\r\n"
    "Pragma: no-cache\r\n"
    "\r\n"
    "<html>"
      "<head><title>"
        "ArtNet Node"
      "</title></head>"
      "<body>"
        "<h1>ArtNet Configuration</h1>"
        "<a href='/'>Back</a><p/>"
        "<form action='/artnet'>"
          "Short Name:<input type='text' name='shortname' value='%s'><p/>"
          "Long Name:<input type='text' name='longname' value='%s'><p/>"
          "ArtNet Subnet:<input type='text' name='subnet' value='%d'><p/>"
          "Universe:<input type='text' name='universe' value='%d'><p/>"
          "<input type='submit'>"
        "</form>"
      "</body>"
    "</html>";

void sendHomePage() {
  unsigned short len = sprintf_P((char*)ether.tcpOffset(), statusPage,
                                 artnet.GetPacketCount(),
                                 artnet.GetFailCount());
  ether.httpServerReply(len);
}

void sendIPPage() {
  unsigned short len = sprintf_P((char*)ether.tcpOffset(), ipPage,
                                 config.iptype == DHCP ? " selected='selected'" : "",
                                 config.iptype == PRIMARY ? " selected='selected'" : "",
                                 config.iptype == SECONDARY ? " selected='selected'" : "",
                                 config.iptype == CUSTOM ? " selected='selected'" : "",
                                 ether.myip[0], ether.myip[1], ether.myip[2], ether.myip[3]);
  ether.httpServerReply(len);
}

void sendArtNetPage() {
  char shortName[19] = {0};
  char longName[65] = {0};
  artnet.GetShortName(shortName);
  artnet.GetLongName(longName);
  unsigned short len = sprintf_P((char*)ether.tcpOffset(), artnetPage,
                                 shortName,
                                 longName,
                                 artnet.GetSubnet(),
                                 artnet.GetInputUniverse(0));
  ether.httpServerReply(len);
}

static int getIntArg(const char* data, const char* key, int value =-1) {
  char temp[10];
  if (ether.findKeyVal(data, temp, sizeof temp, key) > 0)
    value = atoi(temp);
  return value;
}

static void setIpArg(const char *data, const char *key) {
  char ip[16];
  if (ether.findKeyVal(data, ip, sizeof(ip), key) > 0) {
    byte i;
    byte parsedIp[4];
    char *p = strtok(ip, ".");
    if (p == NULL) return;
    parsedIp[0] = atoi(p);
    for (i = 1; i < 4; ++i) {
      p = strtok(NULL, ".");
      if (p == NULL) return;
      parsedIp[i] = atoi(p);
    }
    memcpy(config.ip, parsedIp, 4);
  }
}

static void setShortName(const char *data, const char *key) {
  char shortName[19];
  if (ether.findKeyVal(data, shortName, sizeof(shortName), key) > 0) {
    ether.urlDecode(shortName);
    artnet.SetShortName(shortName);
  }
}

static void setLongName(const char *data, const char *key) {
  char longName[65];
  if (ether.findKeyVal(data, longName, sizeof(longName), key) > 0) {
    ether.urlDecode(longName);
    artnet.SetLongName(longName);
  }
}

int main(void)
{
	init();

        loadConfig();

        // Starting Ethernet
        if (ether.begin(sizeof(Ethernet::buffer), mymac) == 0)
        {
          // Do some mumbo jumbo to warn user of the error
          // leds certainly
        }

        // Configure IP address
        if (config.iptype == DHCP)
        {
          if (!ether.dhcpSetup())
          {
            // Warn user through leds ?
          }
        }
        else if (config.iptype == CUSTOM)
        {
          ether.staticSetup(config.ip, config.gateway);
        }
        else if (config.iptype == PRIMARY)
        {
          byte ip[] = {2, (byte)(mymac[3]+OEM_HI+OEM_LO), mymac[4], mymac[5]};
          byte gwy[] = {2, 0, 0, 1};
          ether.staticSetup(ip, gwy);
        }
        else if (config.iptype == SECONDARY)
        {
          byte ip[] = {10, (byte)(mymac[3]+OEM_HI+OEM_LO), mymac[4], mymac[5]};
          byte gwy[] = {10, 0, 0, 1};
          ether.staticSetup(ip, gwy);
        }
        else
        {
        }

        artnet.Configure(config.iptype == DHCP, ether.myip, ether.broadcastip);

        // Register listener
        ether.enableBroadcast();
        ether.udpServerListenOnPort(&artnetPacket, UDP_PORT_ARTNET);

        // Start DMX
        DMXSerial.init(DMXController);
        DMXSerial.maxChannel(DMXSERIAL_MAX);
                
	for (;;) {
          word pos = 0;
          if ((pos = ether.packetLoop(ether.packetReceive())))
          {
            if (strncmp("GET / ", (const char *)(Ethernet::buffer + pos), 6) == 0)
            {
              // Page emmited
              sendHomePage();
            } else if (strncmp("GET /ip ", (const char *)(Ethernet::buffer + pos), 8) == 0) {
              // Page emmited
              sendIPPage();
            } else if (strncmp("GET /artnet ", (const char *)(Ethernet::buffer + pos), 12) == 0) {
              // Page emmited
              sendArtNetPage();
            } else if (strncmp("GET /artnet?", (const char *)(Ethernet::buffer + pos), 12) == 0) {
              // Save settings
              artnet.SetInputUniverse(0, getIntArg((const char *)(Ethernet::buffer + pos + 11), "universe", artnet.GetInputUniverse(0)));
              artnet.SetSubnet(getIntArg((const char *)(Ethernet::buffer + pos + 11), "subnet", artnet.GetSubnet()));
              setShortName((const char *)(Ethernet::buffer + pos + 11), "shortname");
              setLongName((const char *)(Ethernet::buffer + pos + 11), "longname");
              
              // Send page with new settings
              sendArtNetPage();
            } else if (strncmp("GET /ip?", (const char *)(Ethernet::buffer + pos), 8) == 0) {
              // Save settings
              config.iptype = (IPConfiguration)getIntArg((const char *)(Ethernet::buffer + pos + 7), "iptype", config.iptype);
              setIpArg((const char *)(Ethernet::buffer + pos + 7), "ip");
              saveConfig();
              
              // Send page with new settings
              sendIPPage();
              // Restart the chip to load the new configuration
              resetFunc();
            }
          }
          if (serialEventRun) serialEventRun();
	}
        
	return 0;
}

