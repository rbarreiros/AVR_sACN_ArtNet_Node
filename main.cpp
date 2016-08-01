#include <Arduino.h>
#include "Application.h"

//#include "browser_pages.h"



/*
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
*/

// Reset Function
/*
void(* resetFunc) (void) = 0;

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
*/

/*
static void artnetPacket(unsigned int port, unsigned char ip[4], unsigned int what, const char *data, unsigned int len)
{
  artnet.ProcessPacket(ip, port, data, len);
}

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
*/

Application app;

int main(void)
{
	init();
        
        app.begin();
        
	for (;;) {
          app.loop();

          /*
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
          */
          if (serialEventRun) serialEventRun();
	}
        
	return 0;
}

