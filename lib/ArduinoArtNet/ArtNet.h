/*
  ArtNet Library written for Arduino
  by Chris Staite, yourDream
  Copyright 2013
  
  Modified by Rui Barreiros <rbarreiros@gmail.com>
  Copyright 2016

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ARTNET_H
#define ARTNET_H

#include <Arduino.h>
#include <EtherCard.h>
#include "ArtNetDefs.h"

/* Node defines 
 * this should be defaults, that can be customized from user code
 */

/* Maximum number of ports supported by the node
 * Technically, protocol states each IP should have 4 ports only
 */
#define MAX_PORTS    4
#define ARTNET_PORTS 4

/* OEM code
 * We don't have one so we use 0xffff
 * for example, Simon ArtNet node is 0x0431
 */

#define OEM_HI 0xff
#define OEM_LO 0xff

#define EEPROM_CONFIG 253

typedef struct
{
  uint8_t shortName[18];
  uint8_t longName[64];
  uint8_t subnet;
  uint8_t inputEnable[4];
  uint8_t inUniverse[4];
  uint8_t outUniverse[4];
  uint8_t ip[4];
  uint8_t netmask[4];
  uint16_t port;
  uint8_t magic;
} ArtNetConfig;

/*
 * 
 * 
 */

class ArtNet
{
public:
  ArtNet(uint8_t eepromaddress, uint8_t cfg_ports, bool is_dhcp);
  void configureNetwork(IPConfiguration type);
  
  /*
  ArtNet(byte *mac, byte eepromaddress, byte *buffer, word buflen,
         void (*setIP)(IPConfiguration, const char*, const char*),
         void (*sendFunc)(size_t, word, byte*, word),
         void (*callback)(unsigned short, const char *, unsigned short),
         unsigned char ports);
  */
  
  //void Configure(byte dhcp, byte *ip, byte *broadcastip);

  //ArtNetPortType PortType(unsigned char port);
  //void PortType(unsigned char port, ArtNetPortType type);

  void processPacket(uint8_t ip[4], uint16_t port, const char *data, uint16_t len);

  void getLongName(char *longName);
  void setLongName(char *longName);
  void getShortName(char *shortName);
  void setShortName(char *shortName);
  uint8_t getInputUniverse(uint8_t port);
  void setInputUniverse(uint8_t port, uint8_t universe);
  uint8_t getSubnet();
  void setSubnet(uint8_t subnet);
  uint16_t getPacketCount();
  uint16_t getFailCount();
  
  void sendPoll(bool force);

private:
  void saveConfig(void);
  bool loadConfig(void);
  void setDefaults(void);
  
  void processPoll(byte ip[4], word port, const char *data, word len);
  void processAddress(byte ip[4], word port, const char *data, word len);
  void processIPProg(byte ip[4], word port, const char *data, word len);
  void sendIPProgReply(byte ip[4], word port);
  void processInput(byte ip[4], word port, const char *data, word len);
  void processOutput(byte ip[4], word port, const char *data, word len);

  ArtNetConfig cfg;
  uint8_t artnetDiagnosticPriority;
  uint8_t artnetDiagnosticStatus;
  uint8_t artnetCounter;
  ArtNetStatus_t artnetStatus;
  char *artnetStatusString;  
  uint16_t artnetInCounter;
  uint16_t artnetFailCounter;
  uint8_t eepromAddress;
  bool dhcp;
  uint8_t ports;
  uint8_t artnetInputPortStatus[MAX_PORTS];
  uint8_t artnetOutputPortStatus[MAX_PORTS];

  uint8_t *buffer;

  void (*sendFunc)(size_t length, word sport, byte *dip, word dport);
  void (*callback)(unsigned short, const char *, unsigned short);
  void (*setIP)(IPConfiguration, const char*, const char*);

};

#endif
