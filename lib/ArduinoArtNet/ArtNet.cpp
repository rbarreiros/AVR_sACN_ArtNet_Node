/*
    ArtNet Library written for Arduino
    by Chris Staite, yourDream
    Copyright 2013

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

#include "ArtNet.h"
#include <EEPROM.h>

/**************************************************************************
 * Defines
 **************************************************************************/

#define htons(x) ( ((x)<<8) | (((x)>>8)&0xFF) )

// EEPROM Addreses

#define EEPROM_MAGIC 0
#define EEPROM_ARTNET_SUBNET 1 + 18 + 64
#define EEPROM_ARTNET_IN_UNIVERSE_1 1 + 18 + 64 + 2 + 1
#define EEPROM_ARTNET_IN_UNIVERSE_2 1 + 18 + 64 + 2 + 2
#define EEPROM_ARTNET_IN_UNIVERSE_3 1 + 18 + 64 + 2 + 3
#define EEPROM_ARTNET_IN_UNIVERSE_4 1 + 18 + 64 + 2 + 4
#define EEPROM_ARTNET_OUT_UNIVERSE_1 1 + 18 + 64 + 2 + MAX_PORTS + 1
#define EEPROM_ARTNET_OUT_UNIVERSE_2 1 + 18 + 64 + 2 + MAX_PORTS + 2
#define EEPROM_ARTNET_OUT_UNIVERSE_3 1 + 18 + 64 + 2 + MAX_PORTS + 3
#define EEPROM_ARTNET_OUT_UNIVERSE_4 1 + 18 + 64 + 2 + MAX_PORTS + 4
#define EEPROM_ARTNET_IN_ENABLE_1 1 + 18 + 64 + 2 + MAX_PORTS + MAX_PORTS + 1
#define EEPROM_ARTNET_IN_ENABLE_2 1 + 18 + 64 + 2 + MAX_PORTS + MAX_PORTS + 2
#define EEPROM_ARTNET_IN_ENABLE_3 1 + 18 + 64 + 2 + MAX_PORTS + MAX_PORTS + 3
#define EEPROM_ARTNET_IN_ENABLE_4 1 + 18 + 64 + 2 + MAX_PORTS + MAX_PORTS + 4

#define EEPROM_ARTNET_SHORTNAME + 1
#define EEPROM_ARTNET_LONGNAME + 1 + 18

// IpProg Command

#define ARTNET_IPPROG_CMD_PORT        0x01
#define ARTNET_IPPROG_CMD_SMASK       0x02
#define ARTNET_IPPROG_CMD_IP          0x04
#define ARTNET_IPPROG_CMD_NET_DEFAULT 0x08
#define ARTNET_IPPROG_CMD_ENABLE_DHCP 0x40
#define ARTNET_IPPROG_CMD_ENABLE_PROG 0x80

// IpProgReply Status

#define ARTNET_IPPROGREPLY_STATUS_DHCP_EN 0x40

// Reply Status

#define ARTNET_STATUS_UBEA_PRESENT   0x01
#define ARTNET_STATUS_RDM_CAPABLE    0x02
#define ARTNET_STATUS_BOOT_ROM       0x04
#define ARTNET_STATUS_PA_FRONT_PANEL 0x10
#define ARTNET_STATUS_NETWORK        0x20
#define ARTNET_STATUS_ID_LOCATE      0x40
#define ARTNET_STATUS_ID_MUTE        0x80
#define ARTNET_STATUS_ID_NORMAL      0xc0

// Port types

#define ARTNET_PT_DMX512    0x00
#define ARTNET_PT_MIDI      0x01
#define ARTNET_PT_AVAB      0x02
#define ARTNET_PT_CMX       0x03
#define ARTNET_PT_ADB       0x04
#define ARTNET_PT_ARTNET    0x05
#define ARTNET_PT_OUTARTNET 0x40
#define ARTNET_PT_INARTNET  0x80

// good input

#define ARTNET_GI_ERRORS     0x04
#define ARTNET_GI_INDISABLED 0x08
#define ARTNET_GI_INC_TEXT   0x10
#define ARTNET_GI_INC_SIP    0x20
#define ARTNET_GI_INC_TEST   0x40
#define ARTNET_GI_DATA_RECV  0x80

// good output

#define ARTNET_GO_MERGE_LTP 0x02
#define ARTNET_GO_SHORT     0x04
#define ARTNET_GO_MERGING   0x08
#define ARTNET_GO_INC_TEXT  0x10
#define ARTNET_GO_INC_SIP   0x20
#define ARTNET_GO_INC_TEST  0x40
#define ARTNET_GO_DATA_SNT  0x80

// Status 2

#define ARTNET_STATUS2_WEBCONFIG    0x01
#define ARTNET_STATUS2_DHCP_IP      0x02
#define ARTNET_STATUS2_DHCP_CAPABLE 0x04
#define ARTNET_STATUS2_15BIT_PA     0x08

/**************************************************************************
 * Types
 **************************************************************************/

// Node Styles
typedef enum
{
  ARTNET_NODE = 0,
  ARTNET_CONTROLLER,
  ARTNET_MEDIA,
  ARTNET_ROUTER,
  ARTNET_BACKUP,
  ARTNET_CONFIG,
  ARTNET_VISUALIZER
}  ArtNetNodeStyle;

typedef enum ArtNetOpCodeTag
{
  ARTNET_OP_POLL = 0x2000,
  ARTNET_OP_POLL_REPLY = 0x2100,
  ARTNET_OP_DIAG_DATA = 0x2300,
  ARTNET_OP_OUTPUT = 0x5000,
  ARTNET_OP_ADDRESS = 0x6000,
  ARTNET_OP_INPUT = 0x7000,
  ARTNET_OP_TOD_REQUEST = 0x8000,
  ARTNET_OP_TOD_DATA = 0x8100,
  ARTNET_OP_TOD_CONTROL = 0x8200,
  ARTNET_OP_RDM = 0x8300,
  ARTNET_OP_RDM_SUB = 0x8400,
  ARTNET_OP_VIDEO_SETUP = 0xa010,
  ARTNET_OP_VIDEO_PALETTE = 0xa020,
  ARTNET_OP_VIDEO_DATA = 0xa040,
  ARTNET_OP_MAC_MASTER = 0xf000,
  ARTNET_OP_MAC_SLAVE = 0xf100,
  ARTNET_OP_FIRMWARE_MASTER = 0xf200,
  ARTNET_OP_FIRMWARE_REPLY = 0xf300,
  ARTNET_OP_IP_PROG = 0xf800,
  ARTNET_OP_IP_PROG_REPLY = 0xf900,
  ARTNET_OP_MEDIA = 0x9000,
  ARTNET_OP_MEDIA_PATCH = 0x9100,
  ARTNET_OP_MEDIA_CONTROL = 0x9200,
  ARTNET_OP_MEDIA_CONTROL_REPLY = 0x9300,
  ARTNET_OP_TIMECODE = 0x9700,
} ArtNetOpCode;

typedef struct
{
  ArtNetOpCode opcode;
  uint8_t protocol_hi;
  uint8_t protocol_lo;
} artnetheader_t;

typedef enum ArtNetPriorityTag
{
  ARTNET_DIAGNOSTIC_LOW = 0x10,
  ARTNET_DIAGNOSTIC_MED = 0x40,
  ARTNET_DIAGNOSTIC_HIGH = 0x80,
  ARTNET_DIAGNOSTIC_CRITICAL = 0xe0,
  ARTNET_DIAGNOSTIC_VOLATILE = 0xff
} ArtNetPriority;

// This was wrong !!
typedef enum ArtNetTalkToMeTag
{
  ARTNET_DIAGNOSTIC_ALWAYS = (1 << 1), // <- if bit = 1 whenever node conditions change, send poll reply
                                       //  |- if bit = 0 send poll reply when an ArtPoll or ArtAddress is sent
  ARTNET_DIAGNOSTIC_SEND = (1 << 2),   // <- send or not diagnostic messages (1 true, 0 false)
  ARTNET_DIAGNOSTIC_UNICAST = (1 << 3),  // <- if bit id 1 send unicast, if it's 0 send broadcast, text was misleading
}
ArtNetTalkToMe;

typedef struct
{
  uint8_t  id[8];
  uint16_t opCode;
  uint8_t  verH;
  uint8_t  ver;
  uint8_t  ttm;
  uint8_t  priority;
} artnetpoll_t;

typedef struct
{
  uint8_t  id[8];
  uint16_t opCode;
  uint8_t  ip[4];
  uint16_t port;
  uint8_t  verH;
  uint8_t  ver;
  uint8_t  subH;
  uint8_t  sub;
  uint8_t  oemH;
  uint8_t  oem;
  uint8_t  ubea;
  uint8_t  status;
  uint16_t  estaman;
  uint8_t  shortname[18];
  uint8_t  longname[64];
  uint8_t  nodereport[64];
  uint8_t  numbportsH;
  uint8_t  numbports;
  uint8_t  porttypes[4];
  uint8_t  goodinput[4];
  uint8_t  goodoutput[4];
  uint8_t  swin[4];
  uint8_t  swout[4];
  uint8_t  swvideo;
  uint8_t  swmacro;
  uint8_t  swremote;
  uint8_t  sp1;
  uint8_t  sp2;
  uint8_t  sp3;
  uint8_t  style;
  uint8_t  mac[6];
  uint8_t  bindIp[4];
  uint8_t  bindIndex;
  uint8_t  status2;
  uint8_t  filler[26];
} artnetpollreply_t;

typedef struct
{
  uint8_t  id[8];
  uint16_t opCode;
  uint8_t  verH;
  uint8_t  ver;
  uint16_t filler;
  uint8_t  command;
  uint8_t  filler4;
  uint8_t  progIp[4];
  uint8_t  progSm[4];
  uint8_t  progPortH;
  uint8_t  progPort;
  uint8_t spare[8];
} artnetipprog_t;

typedef struct
{
  uint8_t  id[8];
  uint16_t opCode;
  uint8_t  verH;
  uint8_t  ver;
  uint8_t  filler[4];
  uint8_t  progIp[4];
  uint8_t  progSm[4];
  uint8_t  progPortH;
  uint8_t  progPort;
  uint8_t  status;
  uint8_t  spare[7];
} artnetipprogreply_t;

typedef struct
{
  uint8_t  id[8];
  uint16_t opCode;
  uint8_t  verH;
  uint8_t  ver;
  uint8_t  netSwitch;
  uint8_t  filler2;
  uint8_t  shortName[18];
  uint8_t  longName[64];
  uint8_t  swin[4];
  uint8_t  swout[4];
  uint8_t  subSwitch;
  uint8_t  swVideo;
  uint8_t  command;
} artnetaddress_t;

static char ARTNET_STATUS_STRING_OK[] = "Node Ok";
static char ArtNetMagic[] = "Art-Net";

/* Implementation */

ArtNet::ArtNet(byte *mac, byte eepromaddress, byte *buffer, word buflen,
               void (*setIP)(IPConfiguration, const char*, const char*),
               void (*sendFunc)(size_t, word, byte*, word),
               void (*callback)(unsigned short, const char *, unsigned short),
               unsigned char ports)
{
    if (ports > MAX_PORTS) {
        ports = MAX_PORTS;
    }
    
    unsigned char i;
    unsigned char v;
    
    this->broadcastIP[0] = 255;
    this->broadcastIP[1] = 255;
    this->broadcastIP[2] = 255;
    this->broadcastIP[3] = 255;
    this->serverIP[0] = 255;
    this->serverIP[1] = 255;
    this->serverIP[2] = 255;
    this->serverIP[3] = 255;
    this->mac = mac;
    this->eepromaddress = eepromaddress;
    
    this->buffer = buffer;
    this->buflen = buflen;
    this->sendFunc = sendFunc;
    this->callback = callback;
    this->setIP = setIP;
    
    this->ArtNetDiagnosticPriority = ARTNET_DIAGNOSTIC_CRITICAL;
    this->ArtNetDiagnosticStatus = ARTNET_DIAGNOSTIC_SEND | ARTNET_DIAGNOSTIC_ALWAYS;
    this->ArtNetCounter = 0;
    this->ArtNetStatus = ARTNET_STATUS_POWER_OK;
    this->ArtNetStatusString = ARTNET_STATUS_STRING_OK;

    v = EEPROM.read(eepromaddress + EEPROM_MAGIC);
    if (v != 253) EEPROM.update(eepromaddress + EEPROM_MAGIC, 253);
    if (v != 253) EEPROM.update(eepromaddress + 1 + 18 + 64, 0);

    this->ArtNetSubnet = EEPROM.read(eepromaddress + 1 + 18 + 64);
    if (this->ArtNetSubnet == 0xff) this->ArtNetSubnet = 0;
    
    this->ArtNetInCounter = 0;
    this->ArtNetFailCounter = 0;
    
    memset(this->ArtNetInputPortStatus, 0, MAX_PORTS);
    memset(this->ArtNetOutputPortStatus, 0, MAX_PORTS);
    
    for (i = 0; i < MAX_PORTS; ++i) {
        if (v != 253) EEPROM.update(eepromaddress + 1 + 18 + 64 + 2 + i, i);
        this->ArtNetInputUniverse[i] = EEPROM.read(eepromaddress + 1 + 18 + 64 + 2 + i);
        if (v != 253) EEPROM.update(eepromaddress + 1 + 18 + 64 + 2 + MAX_PORTS + i, i);
        this->ArtNetOutputUniverse[i] = EEPROM.read(eepromaddress + 1 + 18 + 64 + 2 + MAX_PORTS + i);
        if (v != 253) {
            if (i < ports) {
                EEPROM.update(eepromaddress + 1 + 18 + 64 + 2 + MAX_PORTS + MAX_PORTS + i, ARTNET_IN);
            } else {
                EEPROM.update(eepromaddress + 1 + 18 + 64 + 2 + MAX_PORTS + MAX_PORTS + i, ARTNET_OFF);
            }
        }
        this->ArtNetInputEnable[i] = (ArtNetPortType)EEPROM.read(eepromaddress + 1 + 18 + 64 + 2 + MAX_PORTS + MAX_PORTS + i);
    }

    this->Ports = ports;
    
    /* Clear names if uninitialised */
    if (v != 253) {
        SetShortName((char*)NODE_NAME);
        SetLongName((char*)NODE_NAME);
    }
}

ArtNetPortType ArtNet::PortType(unsigned char port)
{
    if (port > MAX_PORTS) return ARTNET_OFF;
    return ArtNetInputEnable[port];
}

void ArtNet::PortType(unsigned char port, ArtNetPortType type)
{
    if (port > MAX_PORTS) return;
    ArtNetInputEnable[port] = type;
}

void ArtNet::Configure(byte dhcp, byte* ip, byte* broadcastip)
{
  unsigned char i;
    
  this->ip = ip;
  this->dhcp = dhcp;
  
  memcpy(&this->broadcastIP, broadcastip, 4);
  
  if (EEPROM.read(eepromaddress + 1 + 18 + 64 + 1) == 1) {
    // Reboot due to IP change
    EEPROM.update(eepromaddress + 1 + 18 + 64 + 1, 0);
    byte sendIp[4];
    word sendPort;
    for (i = 0; i < 4; ++i) {
      sendIp[i] = EEPROM.read(eepromaddress + 1 + 18 + 64 + 2 + MAX_PORTS * 3 + i);
    }
    for (i = 0; i < 2; ++i) {
      ((byte*)&sendPort)[i] = EEPROM.read(eepromaddress + 1 + 18 + 64 + 2 + MAX_PORTS * 3 + 4 + i);
    }
    this->sendIPProgReply(sendIp, sendPort);
  } else {
    // Standard boot
    this->SendPoll(1);
  }
}

void ArtNet::GetShortName(char *shortName)
{
    unsigned char i;
    for (i = 0; i < 18; ++i) {
        shortName[i] = EEPROM.read(this->eepromaddress + 1 + i);
    }
}

void ArtNet::SetShortName(char *shortName)
{
    unsigned char i;
    for (i = 0; i < 18; ++i) {
        EEPROM.update(this->eepromaddress + 1 + i, shortName[i]);
        if (!shortName[i]) break;
    }
    for (; i < 18; ++i) {
        EEPROM.update(this->eepromaddress + 1 + i, 0);
    }
}

void ArtNet::GetLongName(char *longName)
{
    unsigned char i;
    for (i = 0; i < 64; ++i) {
        longName[i] = EEPROM.read(this->eepromaddress + 1 + 18 + i);
    }
}

void ArtNet::SetLongName(char *longName)
{
    unsigned char i;
    for (i = 0; i < 64; ++i) {
        EEPROM.update(this->eepromaddress + 1 + 18 + i, longName[i]);
        if (!longName[i]) break;
    }
    for (; i < 64; ++i) {
        EEPROM.update(this->eepromaddress + 1 + 18 + i, 0);
    }
}

unsigned char ArtNet::GetInputUniverse(unsigned char port)
{
    if (port >= MAX_PORTS) return 0;
    return this->ArtNetInputUniverse[port];
}

void ArtNet::SetInputUniverse(unsigned char port, unsigned char universe)
{
    if (port >= MAX_PORTS) return;
    this->ArtNetInputUniverse[port] = universe;
    EEPROM.update(this->eepromaddress + 1 + 18 + 64 + 2 + port, universe);
}

unsigned char ArtNet::GetSubnet()
{
    return this->ArtNetSubnet;
}

void ArtNet::SetSubnet(unsigned char subnet)
{
    this->ArtNetSubnet = subnet;
    EEPROM.update(this->eepromaddress + 1 + 18 + 64, subnet);
}

unsigned int ArtNet::GetPacketCount()
{
    return this->ArtNetInCounter;
}

unsigned int ArtNet::GetFailCount()
{
    return this->ArtNetFailCounter;
}


void ArtNet::processAddress(byte ip[4], word port, const char *data, word len)
{
  unsigned char i;
	
  // Read data
  data += sizeof(artnetheader_t) + sizeof(ArtNetMagic);

  // Set the short name
  // Check if the name is null
  if (data[2]) {
    // Let's set the short name
    for (i = 0; i < 18; ++i)
      EEPROM.update(this->eepromaddress + 1 + i, data[2 + i]);
  }

  // Set the long name
  // Check if the name is null
  if (data[20]) {
    // Let's set the long name
    for (i = 0; i < 64; ++i)
      EEPROM.update(this->eepromaddress + 1 + 18 + i, data[2 + 18 + i]);
  }
  
  // Set input universes
  for (i = 0; i < MAX_PORTS; i++) {
    if (data[64 + 19 + 1 + i] != 0x7f && (data[64 + 19 + 1 + i] & (1 << 7))) {
      unsigned char t;
      // Only set if bit 7 is high
      t = data[64 + 19 + 1 + i] & ~(1 << 7);
      if (this->ArtNetInputUniverse[i] != t) {
        this->ArtNetInputUniverse[i] = t;
        EEPROM.update(this->eepromaddress + 1 + 18 + 64 + 2 + i, t);
      }
    }
  }
  
  // Set output universes
  for (i = 0; i < MAX_PORTS; i++) {
    if (data[64 + 19 + 1 + ARTNET_PORTS + i] != 0x7f && (data[64 + 19 + 1 + ARTNET_PORTS + i] & (1 << 7))) {
      unsigned char t;
      // Only set if bit 7 is high
      t = data[64 + 19 + 1 + ARTNET_PORTS + i] & ~(1 << 7);
      if (this->ArtNetOutputUniverse[i] != t) {
        this->ArtNetOutputUniverse[i] = t;
        EEPROM.update(this->eepromaddress + 1 + 18 + 64 + 2 + MAX_PORTS + i, t);
      }
    }
  }
  
  // Set subnet
  if (data[64 + 19 + 1 + ARTNET_PORTS + ARTNET_PORTS] != 0x7f && data[64 + 19 + 1 + ARTNET_PORTS + ARTNET_PORTS] & (1 << 7)) {
    unsigned char t;
    t = data[64 + 19 + 1 + ARTNET_PORTS + ARTNET_PORTS] & ~(1 << 7);
    if (this->ArtNetSubnet != t) {
      this->ArtNetSubnet = t;
      EEPROM.update(this->eepromaddress + 1 + 18 + 64, t);
    }
  }
  
  // Command - mostly ignored because we don't support any merging
  /*
  switch ((char)(data[64 + 19 + 1 + ARTNET_PORTS + ARTNET_PORTS + 2])) {
    case 0x90:
    case 0x91:
    case 0x92: {
      unsigned char t;
      t = data[64 + 19 + 1 + ARTNET_PORTS + ARTNET_PORTS + 2] & 0x3;
      // Reset data on port t
      break;
    }
  }
  */

  this->SendPoll(1);
}

void ArtNet::processInput(byte ip[4], word port, const char *data, word len)
{
	// Read data
	data += sizeof(artnetheader_t) + sizeof(ArtNetMagic);
	unsigned char i;
	
	for (i = 0; i < MAX_PORTS; i++) {
		if (this->ArtNetInputEnable[i] != (data[4 + i] & 1)) {
			// Configure as input
			if (this->ArtNetInputEnable[i] != (data[4 + i] & 1)) {
				this->ArtNetInputEnable[i] = (data[4 + i] & 1) ? ARTNET_OUT : ARTNET_IN;
				EEPROM.update(this->eepromaddress + 1 + 18 + 64 + MAX_PORTS + MAX_PORTS + i, this->ArtNetInputEnable[i]);
			}
			// Reconfigure port
			if (data[4 + i] & 1) {
			    // Port i - Input
			} else {
			    // Port i - Output
			}
		}
	}
}

/**
 * ArtIpProgReply - TODO
 *
 * ----------------------------------------------------------------------------
 * Entity     | Direction        | Action
 * ----------------------------------------------------------------------------
 * Controller | Receive          | No action
 *            | Unicast Transmit | Not Allowed
 *            | Broadcast        | Not Allowed
 * ----------------------------------------------------------------------------
 * Node       | Receive          | No Action
 *            | Unicast Transmit | Transmits to specific Controller IP address.
 *            | Broacast         | Not Allowed
 * ----------------------------------------------------------------------------
 * Media      | Receive          | No Action
 * Server     | Unicast Transmit | Transmits to specific Controller IP address.
 *            | Broadcast        | Not Allowed
 * ----------------------------------------------------------------------------
 */

void ArtNet::sendIPProgReply(byte ip[4], word port)
{
    size_t length = 0;
    unsigned short t16;

    // Magic
    memcpy(this->buffer, ArtNetMagic, sizeof(ArtNetMagic));
    length += sizeof(ArtNetMagic);

    // Op code
    t16 = htons(ARTNET_OP_IP_PROG_REPLY);
    memcpy(&this->buffer[length], &t16, 2);
    length += 2;

    // Version
    t16 = htons(14);
    memcpy(&this->buffer[length], &t16, 2);
    length += 2;

    // Padding
    memset(&this->buffer[length], 0, 4);
    length += 4;

    // Node IP
    memcpy(&this->buffer[length], this->ip, 4);
    length += 4;
    
    // Node subnet
    memset(&this->buffer[length], 0, 4);
    length += 4;

    // Port
    t16 = htons(UDP_PORT_ARTNET);
    memcpy(&this->buffer[length], &t16, 2);
    length += 2;
    
    // Status (DHCP enabled?)
    this->buffer[length++] = this->dhcp;
    
    // Spare/Filler
    memset(&this->buffer[length], 0, 7);
    length += 7;

    // Transmit ArtNetIpProgReply
    this->sendFunc(length, UDP_PORT_ARTNET, ip, port);
}

void ArtNet::processIPProg(byte ip[4], word port, const char *data, word len)
{
    unsigned char i;
    IPConfiguration type;
    const char *newip = 0;
    const char *subnet = 0;
    
	// Read data
	data += sizeof(artnetheader_t) + sizeof(ArtNetMagic);
	
	// Process command
	if (!(data[2] & (1 << 7))) {
		// No programming enabled
		this->sendIPProgReply(ip, port);
		return;
	}
	
	if (data[2] & (1 << 6)) {
		// Enable DHCP
		if (this->dhcp != 1)
			type = DHCP;
	}
	
	if (data[2] & (1 << 3)) {
		// Set to default
		type = PRIMARY;
	}
	
	// Read four bytes
	if (data[2] & (1 << 2)) {
		newip = data + 4;
	}

	if (data[2] & (1 << 1)) {
		subnet = data + 8;
	}
	
	if (data[2] & 1) {
		// Program port - ignore this for now
	}
	
	// Set eeprom bit
	EEPROM.update(this->eepromaddress + 1 + 18 + 64 + 1, 1);
	for (i = 0; i < 4; ++i) {
	    EEPROM.update(this->eepromaddress + 1 + 18 + 64 + 2 + MAX_PORTS + MAX_PORTS + MAX_PORTS + i, ip[i]);
	}
	for (i = 0; i < 2; ++i) {
	    EEPROM.update(this->eepromaddress + 1 + 18 + 64 + 2 + MAX_PORTS + MAX_PORTS + MAX_PORTS + 4 + i, ((byte*)&port)[i]);
	}
	// Save (and reboot)
	this->setIP(type, newip, subnet);
}

void ArtNet::ProcessPacket(byte ip[4], word port, const char *data, word len)
{
  artnetheader_t *header;
	
  if (strncmp(data, ArtNetMagic, 7) != 0) {
    this->ArtNetFailCounter++;
    return;
  }
	
  this->ArtNetInCounter++;
        
  header = (artnetheader_t*)(data + sizeof(ArtNetMagic));
    
  if (header->protocol_lo < 14) {
    return;
  }

  switch (header->opcode) {
    /* Input and Configuration */
    	
    case ARTNET_OP_POLL:
      this->processPoll(ip, port, data, len);
      break;
    case ARTNET_OP_OUTPUT:
      {
        unsigned short universe, length;
        unsigned char i;
        
        // Read data
        data += sizeof(artnetheader_t) + sizeof(ArtNetMagic);
	
        universe = data[2] | (data[3] << 8);
        
        // Length
        length = htons(data[4]);
        
        for (i = 0; i < this->Ports; i++) {
          if (this->ArtNetInputUniverse[i] == universe && this->ArtNetInputEnable[i] == ARTNET_IN) {
            // Set Data for this output
            // Port i - d[6 + j] (j = 0 to length)
            this->callback(i, &data[6], length);
          }
        }
      }
      break;
    case ARTNET_OP_ADDRESS:
      this->processAddress(ip, port, data, len);
      break;
    case ARTNET_OP_INPUT:
      this->processInput(ip, port, data, len);
      break;
    case ARTNET_OP_IP_PROG:
      this->processIPProg(ip, port, data, len);
      break;
      
      /* Undocumented - for manufacturer use (that's me!) */
      
    case ARTNET_OP_MAC_MASTER:
      break;
    case ARTNET_OP_MAC_SLAVE:
      break;
      
      /* RDM - Currently unsupported */
      
    case ARTNET_OP_RDM:
      break;
    case ARTNET_OP_RDM_SUB:
      break;
    case ARTNET_OP_TOD_REQUEST:
      break;
    case ARTNET_OP_TOD_DATA:
      break;
    case ARTNET_OP_TOD_CONTROL:
      break;
      
      /* Ignored broadcasted reply op codes */
      
    case ARTNET_OP_POLL_REPLY:
      // Ignore replies
    case ARTNET_OP_IP_PROG_REPLY:
      // Ignore IP Programming replies
    case ARTNET_OP_DIAG_DATA:
      // Ignore diagnostics
      break;
      
      /* Unsupported feature op codes */
      
    case ARTNET_OP_TIMECODE:
      break;
      
    case ARTNET_OP_FIRMWARE_MASTER:
    case ARTNET_OP_FIRMWARE_REPLY:
      // Don't have the capability to do OTW firmware update
      break;
      
    case ARTNET_OP_VIDEO_SETUP:
    case ARTNET_OP_VIDEO_PALETTE:
    case ARTNET_OP_VIDEO_DATA:
      // Ignore video data
      break;
      
    case ARTNET_OP_MEDIA:
    case ARTNET_OP_MEDIA_PATCH:
    case ARTNET_OP_MEDIA_CONTROL:
    case ARTNET_OP_MEDIA_CONTROL_REPLY:
      // Ignore media data
      break;
      
      /* Unknown op code */
      
    default:
      ArtNetStatus = ARTNET_STATUS_PARSE_FAIL;
      this->SendPoll(0);
      return;
  }
}

/**
 * ArtPoll 
 *
 * --------------------------------------------------------------------------------------
 * Entity      | Direction        | Action
 * --------------------------------------------------------------------------------------
 *  Controller | Receive            | Send ArtPollReply
 *             | Unicast Transmit   | Not Allowed.
 *             | Directed Broadcast | Controller broadcasts this packet to poll all
 *             |                    | Controllers and Nodes on the network.
 *  Node       | Receive            | Send ArtPollReply
 *             | Unicode Transmit   | Not Allowed.
 *             | Broadcast          | Not Allowed.
 *  Media      | Receive            | Send ArtPollReply
 *  Server     | Unicast Transmit   | Not Allowed.
 *             | Broadcast          | Not Allowed.
 * ---------------------------------------------------------------------------------------
 */

void ArtNet::processPoll(byte ip[4], word port, const char *data, word len)
{
  artnetpoll_t *poll = (artnetpoll_t *)data;
	
  memcpy(&this->serverIP, ip, 4);
  
  this->ArtNetDiagnosticStatus = poll->ttm;
  this->ArtNetDiagnosticPriority = poll->priority;
  this->SendPoll(1);
}

/**
 * ArtPollReply - 
 *
 * --------------------------------------------------------------------------------------
 * Entity   | Direction        | Action
 * --------------------------------------------------------------------------------------
 *  All     | Receive          | No Art-Net Action
 *  Devices | Unicast Transmit | Not Allowed.
 *          | Broadcast        | Directed Broadcasts this packet in response to an ArtPoll
 * ---------------------------------------------------------------------------------------
 *
 * Directed broadcast is a packet to the network broadcast address, for instance on a 
 * network with IP 192.168.1.X on netmask 255.255.255.0 directed broadcast address is
 * 192.168.1.255
 * Limited broadcast is 255.255.255.255
 */

void ArtNet::SendPoll(unsigned char force)
{
  unsigned int i;
  
  if (!force && !(this->ArtNetDiagnosticStatus & ARTNET_DIAGNOSTIC_ALWAYS)) {
    // We are not forcing (i.e. not replying to ArtPoll) and not always sending updates
    return;
  }
	
  if (!force) {
    // Increment the non-requested poll counter
    this->ArtNetCounter++;
  }

  artnetpollreply_t *reply = (artnetpollreply_t *)buffer;

  memcpy(reply->id, ArtNetMagic, sizeof(ArtNetMagic));
  reply->opCode = ARTNET_OP_POLL_REPLY;
  memcpy(reply->ip, this->ip, 4);
  reply->port = UDP_PORT_ARTNET;

  reply->verH = (ARTNET_VERSION >> 8) & 0xff;
  reply->ver = ARTNET_VERSION & 0xff;
  reply->subH = (this->ArtNetSubnet >> 8) & 0xff;
  reply->sub = this->ArtNetSubnet & 0xff;
  reply->oemH = OEM_HI;
  reply->oem = OEM_LO;
  reply->ubea = 0;

  // Status 1
  // Indicators in Normal mode
  // Universe programmed by network
  reply->status = ARTNET_STATUS_ID_NORMAL |
      ARTNET_STATUS_NETWORK;
  
  // ESTA (YD)
  //t16 = 0x5944;
  //t16 = 0x7a70;
  reply->estaman = 0x00;

  // Names
  {
    // Short name (18 bytes)
    for (i = 0; i < 18; i++)
      reply->shortname[i] = EEPROM.read(this->eepromaddress + 1 + i);
    reply->shortname[17] = '\0';
    
    // Long name (64 bytes)
    for (i = 0; i < 64; i++)
      reply->longname[i] = EEPROM.read(this->eepromaddress + 1 + 18 + i);
    reply->longname[63] = '\0';
  }
  
  // Report
  snprintf((char*)&reply->nodereport, 64, "#%x %d %s", this->ArtNetStatus, this->ArtNetCounter, this->ArtNetStatusString);
  
  // Number of DMX ports
  reply->numbportsH = (this->Ports >> 8) & 0xff;
  reply->numbports = this->Ports & 0xff;
  
  // Port Configuration
  // Port 1-4
  for (i = 0; i < 4; i++) {
    reply->porttypes[i] = (i <= this->Ports) ? (ARTNET_PT_DMX512 | ARTNET_PT_INARTNET) : 0;
    reply->goodinput[i] = (i <= this->Ports) ? ArtNetInputPortStatus[i] : 0;
    reply->goodoutput[i] = (i <= this->Ports) ? ArtNetOutputPortStatus[i] : 0;
    reply->swin[i] = (i <= this->Ports) ? ArtNetInputUniverse[i] : 0;
    reply->swout[i] = (i <= this->Ports) ? ArtNetOutputUniverse[i] : 0;
  }
  
  reply->swvideo = 0;
  reply->swmacro = 0;
  reply->swremote = 0;
  reply->sp1 = 0;
  reply->sp2 = 0;
  reply->sp3 = 0;

  reply->style = ARTNET_NODE;
  
  // MAC Address
  memcpy(&reply->mac, this->mac, 6);
  
  // Bind IP, set to the same as self IP
  memcpy(&reply->bindIp, this->ip, 4);
  
  // Bind Index - Root node, so 0
  reply->bindIndex = 0;

  // Status 2
  reply->status2 = ARTNET_STATUS2_WEBCONFIG |
      ARTNET_STATUS2_DHCP_CAPABLE;
  reply->status2 |= (this->dhcp) ? ARTNET_STATUS2_DHCP_IP : 0;

  // Filler
  memset(&reply->filler, 0, 26);
  
  // Transmit ArtNetPollReply
  this->sendFunc(sizeof(artnetpollreply_t), UDP_PORT_ARTNET, this->broadcastIP, UDP_PORT_ARTNET);
  
  // Reset status
  this->ArtNetStatus = ARTNET_STATUS_POWER_OK;
  this->ArtNetStatusString = ARTNET_STATUS_STRING_OK;
}
