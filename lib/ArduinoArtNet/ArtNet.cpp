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

    NOTE:

    This library requires EtherCard library for ethernet access and
    EEPROMEx library for EEPROM access
*/

#include "ArtNet.h"
#include <EEPROMex.h>

static char ARTNET_STATUS_STRING_OK[] = "Node Ok";
static char ArtNetMagic[] = "Art-Net\0";

/* Implementation */

ArtNet::ArtNet(uint8_t eepromaddress, uint8_t cfg_ports, bool is_dhcp)
    : artnetDiagnosticPriority(ARTNET_DIAGNOSTIC_CRITICAL),
      artnetDiagnosticStatus(ARTNET_DIAGNOSTIC_SEND | ARTNET_DIAGNOSTIC_ALWAYS),
      artnetCounter(0),
      artnetStatus(ARTNET_STATUS_POWER_OK),
      artnetStatusString(ARTNET_STATUS_STRING_OK),
      artnetInCounter(0),
      artnetFailCounter(0),
      eepromAddress(eepromaddress),
      dhcp(is_dhcp)
{
  if(cfg_ports > MAX_PORTS)
    cfg_ports = MAX_PORTS;
  
  ports = cfg_ports;

  if(!loadConfig())
    setDefaults();

  buffer = Ethernet::buffer + UDP_DATA_P;

  // Add our Callback
  ether.enableBroadcast();
  //ether.udpServerListenOnPort(&(this->processPacket), UDP_PORT_ARTNET);
  
  sendPoll(true);
}

bool ArtNet::loadConfig(void)
{
  EEPROM.readBlock(eepromAddress, cfg);
  return (cfg.magic == EEPROM_CONFIG);
}

void ArtNet::saveConfig(void)
{
  EEPROM.writeBlock(eepromAddress, cfg);
}

void ArtNet::setDefaults(void)
{
  uint8_t i;
  
  cfg.magic = EEPROM_CONFIG;
  memcpy(cfg.shortName, NODE_NAME, 18);
  memcpy(cfg.longName, NODE_NAME, 64);
  cfg.subnet = 0;

  for(i = 0; i < MAX_PORTS; i++)
  {
    if(i <= ports)
    {
      cfg.inputEnable[i] = 1;
      cfg.inUniverse[i] = 0;
      cfg.outUniverse[i] = 0;
    }
  }
  
  for(i = 0; i < 4; i++)
  {
    cfg.ip[i] = 0xff;
    cfg.netmask[i] = 0xff;
  }

  cfg.port = UDP_PORT_ARTNET;

  saveConfig();
}

void ArtNet::configureNetwork(IPConfiguration type)
{
}

/*
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
        //this->ArtNetInputEnable[i] = (ArtNetPortType)EEPROM.read(eepromaddress + 1 + 18 + 64 + 2 + MAX_PORTS + MAX_PORTS + i);
        this->ArtNetInputEnable[i] = EEPROM.read(eepromaddress + 1 + 18 + 64 + 2 + MAX_PORTS + MAX_PORTS + i);
    }

    this->Ports = ports;
    
    // Clear names if uninitialised
    if (v != 253) {
        SetShortName((char*)NODE_NAME);
        SetLongName((char*)NODE_NAME);
    }
}
*/

/*
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
*/

/**
 * This requires EtherCard already configured!
 */
/*
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
  */

void ArtNet::getShortName(char *shortName)
{
  memcpy(shortName, cfg.shortName, 18);
}

void ArtNet::setShortName(char *shortName)
{
  memcpy(cfg.shortName, shortName, 18);
  saveConfig();
}

void ArtNet::getLongName(char *longName)
{
  memcpy(longName, cfg.longName, 64);
}

void ArtNet::setLongName(char *longName)
{
  memcpy(cfg.longName, longName, 64);
  saveConfig();
}

uint8_t ArtNet::getInputUniverse(uint8_t port)
{
  if (port >= MAX_PORTS) return 0;
  return cfg.inUniverse[port];
}

void ArtNet::setInputUniverse(uint8_t port, uint8_t universe)
{
  if (port >= MAX_PORTS) return;
  cfg.inUniverse[port] = universe;
  saveConfig();
}

uint8_t ArtNet::getSubnet()
{
  return cfg.subnet;
}

void ArtNet::setSubnet(uint8_t subnet)
{
  cfg.subnet = subnet;
  saveConfig();
}

uint16_t ArtNet::getPacketCount()
{
  return this->artnetInCounter;
}

uint16_t ArtNet::getFailCount()
{
  return this->artnetFailCounter;
}

void ArtNet::processPacket(uint8_t ip[4], uint16_t port, const char *data, uint16_t len)
{
  artnetheader_t *header = (artnetheader_t *)data;

  if(strncmp(reinterpret_cast<char*>(header->id), ArtNetMagic, 8) != 0)
  {
    this->artnetFailCounter++;
    return;
  }
	
  this->artnetInCounter++;

  if(ARTNET_VERSION > ((header->verH << 8) | header->ver))
    return;
  
  switch (header->opcode) {
    /* Input and Configuration */
    case ARTNET_OP_POLL:
      this->processPoll(ip, port, data, len);
      break;
    case ARTNET_OP_OUTPUT:
      this->processOutput(ip, port, data, len);
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
      artnetStatus = ARTNET_STATUS_PARSE_FAIL;
      this->sendPoll(0);
      return;
  }
}

/**
 * ArtPoll 
 *
 * --------------------------------------------------------------------------------------
 * Entity      | Direction          | Action
 * --------------------------------------------------------------------------------------
 *  Controller | Receive            | Send ArtPollReply
 *             | Unicast Transmit   | Not Allowed.
 *             | Directed Broadcast | Controller broadcasts this packet to poll all
 *             |                    | Controllers and Nodes on the network.
 *  Node       | Receive            | Send ArtPollReply
 *             | Unicast Transmit   | Not Allowed.
 *             | Broadcast          | Not Allowed.
 *  Media      | Receive            | Send ArtPollReply
 *  Server     | Unicast Transmit   | Not Allowed.
 *             | Broadcast          | Not Allowed.
 * ---------------------------------------------------------------------------------------
 */

void ArtNet::processPoll(byte ip[4], word port, const char *data, word len)
{
  artnetpoll_t *poll = (artnetpoll_t *)data;
	
  //memcpy(&this->serverIP, ip, 4);
  
  this->artnetDiagnosticStatus = poll->ttm;
  this->artnetDiagnosticPriority = poll->priority;
  this->sendPoll(true);
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

void ArtNet::sendPoll(bool force)
{
  unsigned int i;
  
  if (!force && !(this->artnetDiagnosticStatus & ARTNET_DIAGNOSTIC_ALWAYS))
  {
    // We are not forcing (i.e. not replying to ArtPoll) and not always sending updates
    return;
  }
	
  if (!force) {
    // Increment the non-requested poll counter
   this->artnetCounter++;
  }

  artnetpollreply_t *reply = (artnetpollreply_t *)buffer;

  memcpy(reply->id, ArtNetMagic, sizeof(ArtNetMagic));
  reply->opCode = ARTNET_OP_POLL_REPLY;
  memcpy(reply->ip, ether.myip, 4);
  reply->port = UDP_PORT_ARTNET;

  reply->verH = (ARTNET_VERSION >> 8) & 0xff;
  reply->ver = ARTNET_VERSION & 0xff;
  reply->subH = (cfg.subnet >> 8) & 0xff;
  reply->sub = cfg.subnet & 0xff;
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
  memcpy(reply->shortName, cfg.shortName, 18);
  memcpy(reply->longName, cfg.longName, 18);
  
  // Report
  snprintf((char*)&reply->nodereport, 64, "#%x %d %s", this->artnetStatus, this->artnetCounter, this->artnetStatusString);
  
  // Number of DMX ports
  reply->numbportsH = (this->ports >> 8) & 0xff;
  reply->numbports = this->ports & 0xff;
  
  // Port Configuration
  // Port 1-4
  for (i = 0; i < 4; i++) {
    reply->porttypes[i] = (i <= ports) ? (ARTNET_PT_DMX512 | ARTNET_PT_INARTNET) : 0;
    reply->goodinput[i] = (i <= ports) ? artnetInputPortStatus[i] : 0;
    reply->goodoutput[i] = (i <= ports) ? artnetOutputPortStatus[i] : 0;
    reply->swin[i] = (i <= ports) ? cfg.inUniverse[i] : 0;
    reply->swout[i] = (i <= ports) ? cfg.outUniverse[i] : 0;
  }
  
  reply->swvideo = 0;
  reply->swmacro = 0;
  reply->swremote = 0;
  reply->sp1 = 0;
  reply->sp2 = 0;
  reply->sp3 = 0;

  reply->style = ARTNET_NODE;
  
  // MAC Address
  memcpy(&reply->mac, ether.mymac, 6);
  
  // Bind IP, set to the same as self IP
  memcpy(&reply->bindIp, ether.myip, 4);
  
  // Bind Index - Root node, so 0
  reply->bindIndex = 0;

  // Status 2
  reply->status2 = ARTNET_STATUS2_WEBCONFIG |
      ARTNET_STATUS2_DHCP_CAPABLE;
  reply->status2 |= (this->dhcp) ? ARTNET_STATUS2_DHCP_IP : 0;

  // Filler
  memset(&reply->filler, 0, 26);
  
  // Transmit ArtNetPollReply
  this->sendFunc(sizeof(artnetpollreply_t), UDP_PORT_ARTNET, ether.broadcastip, UDP_PORT_ARTNET);
  
  // Reset status
  this->artnetStatus = ARTNET_STATUS_POWER_OK;
  this->artnetStatusString = ARTNET_STATUS_STRING_OK;
}

/**
 * ArtIpProg
 *
 * --------------------------------------------------------------------------------------
 * Entity      | Direction          | Action
 * --------------------------------------------------------------------------------------
 *  Controller | Receive            | No Action
 *             | Unicast Transmit   | Controller transmits to a specific node IP address.
 *             | Directed Broadcast | Not Allowed.
 *  Node       | Receive            | Reply with ArtIpProgReply.
 *             | Unicast Transmit   | Not Allowed.
 *             | Broadcast          | Not Allowed.
 *  Media      | Receive            | Reply with ArtIpProgReply.
 *  Server     | Unicast Transmit   | Not Allowed.
 *             | Broadcast          | Not Allowed.
 * ---------------------------------------------------------------------------------------
 */

void ArtNet::processIPProg(byte ip[4], word port, const char *data, word len)
{
  bool doSaveConfig = false;
  IPConfiguration type;
  artnetipprog_t *prog = (artnetipprog_t *)data;
  
  // Programming enabled ?
  if(!(prog->command & ARTNET_IPPROG_CMD_ENABLE_PROG))
  {
    // If command is clear, it's an inquiry, only then
    // we reply with an ipprogreply, if not we just return
    if(prog->command == 0)
      this->sendIPProgReply(ip, port);
    return;
  }

  if(prog->command & ARTNET_IPPROG_CMD_ENABLE_DHCP)
    if(dhcp != 1)
      type = DHCP;

  // Set to default IP configuration
  // now, which one should it be ?
  // Primary ? Secondary ? user ? hmmm
  if(prog->command & ARTNET_IPPROG_CMD_NET_DEFAULT)
    type = PRIMARY;
  
  if(prog->command & ARTNET_IPPROG_CMD_IP)
  {
    memcpy(cfg.ip, prog->progIp, 4);
    doSaveConfig = true;
  }

  if(prog->command & ARTNET_IPPROG_CMD_SMASK)
  {
    memcpy(cfg.netmask, prog->progSm, 4);
    doSaveConfig = true;
  }

  if(prog->command & ARTNET_IPPROG_CMD_PORT)
  {
    cfg.port = (prog->progPortH << 8) | prog->progPort;
    doSaveConfig = true;
  }

  if(doSaveConfig)
    saveConfig();

  // Change Ethercard network config
  configureNetwork(type);
  
  // Send the reply
  this->sendIPProgReply(ip, port);
}

/**
 * ArtIpProgReply
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
  artnetipprogreply_t *reply = (artnetipprogreply_t *)this->buffer;
  
  // Magic
  memcpy(reply->header.id, ArtNetMagic, sizeof(ArtNetMagic));

  reply->header.opcode = ARTNET_OP_IP_PROG_REPLY;
  reply->header.verH = (ARTNET_VERSION >> 8) & 0xff;
  reply->header.ver = (ARTNET_VERSION & 0xff);
  memset(reply->filler, 0, 4);
  memcpy(reply->progIp, ether.myip, 4);
  // TODO
  memcpy(reply->progSm, ether.netmask, 4);

  reply->progPortH = (UDP_PORT_ARTNET >> 8) & 0xff;
  reply->progPort = (UDP_PORT_ARTNET & 0xff);

  reply->status = (dhcp) ? ARTNET_IPPROGREPLY_STATUS_DHCP_EN : 0;
    
  // Spare/Filler
  memset(reply->spare, 0, 7);

  // Transmit ArtNetIpProgReply
  this->sendFunc(sizeof(artnetipprogreply_t), UDP_PORT_ARTNET, ip, port);
}

/**
 * ArtAddress
 *
 * --------------------------------------------------------------------------------------
 * Entity      | Direction          | Action
 * --------------------------------------------------------------------------------------
 *  Controller | Receive            | No Action
 *             | Unicast Transmit   | Controller transmits to a specific node IP address.
 *             | Directed Broadcast | Not Allowed.
 *  Node       | Receive            | Reply by broadcasting ArtPollReply.
 *             | Unicast Transmit   | Not Allowed.
 *             | Broadcast          | Not Allowed.
 *  Media      | Receive            | Reply by broadcasting ArtPollReply.
 *  Server     | Unicast Transmit   | Not Allowed.
 *             | Broadcast          | Not Allowed.
 * ---------------------------------------------------------------------------------------
 */

void ArtNet::processAddress(byte ip[4], word port, const char *data, word len)
{
  unsigned char i;
  bool doSaveConfig = false;
  artnetaddress_t *addr = (artnetaddress_t*)data;

  if(addr->shortName[0])
  {
    memcpy(cfg.shortName, addr->shortName, 18);
    doSaveConfig = true;
  }

  if(addr->longName[0])
  {
    memcpy(cfg.longName, addr->longName, 64);
    doSaveConfig = true;
  }

  /*
    .NetSwitch
    Bits 14-8 of the 15 bit Port-Address are encoded into
    the bottom 7 bits of this field. This is used in
    combination with SubSwitch and SwIn[] or SwOut[] to
    produce the full universe address.
    This value is ignored unless bit 7 is high. i.e. to
    program a value 0x07, send the value as 0x87.
    
    Send 0x00 to reset this value to the physical switch setting.
    Use value 0x7f for no change.

    // ! TODO !//
   */




  /*
    .swin .swout .subswitch
    Bits 3-0 of the 15 bit Port-Address for a given input
    port are encoded into the bottom 4 bits of this field.
    This is used in combination with NetSwitch and
    SubSwitch to produce the full universe address.
    This value is ignored unless bit 7 is high. i.e. to
    program a value 0x07, send the value as 0x87.
    Send 0x00 to reset this value to the physical switch
    setting.
    Use value 0x7f for no change.
  */
  
  for(i = 0; i < MAX_PORTS; i++)
  {
    // Set Input Universes
    if(addr->swin[i] != 0x7f && (addr->swin[i] & (1 << 7)))
    {
      cfg.inUniverse[i] = addr->swin[i] & ~(1 << 7);
      doSaveConfig = true;
    }

    // Set Output Universes
    if(addr->swout[i] != 0x7f && (addr->swout[i] & (1 << 7)))
    {
      cfg.outUniverse[i] = addr->swout[i] & ~(1 << 7);
      doSaveConfig = true;
    }
  }

  /*
    .subSwitch
    Bits 7-4 of the 15 bit Port-Address are encoded into
    the bottom 4 bits of this field. This is used in
    combination with NetSwitch and SwIn[] or SwOut[] to
    produce the full universe address.

    This value is ignored unless bit 7 is high. i.e. to
    program a value 0x07, send the value as 0x87.
    Send 0x00 to reset this value to the physical switch
    setting.
    Use value 0x7f for no change.
   */
  
  if(addr->subSwitch != 0x7f && (addr->subSwitch & (1 << 7)))
  {
    cfg.subnet = (addr->subSwitch & ~(1 << 7));
    doSaveConfig = true;
  }

  /** Command **/
  // Not Implemented //
  /*
  switch(addr->command)
  {
    
  }
  */
  
  if(doSaveConfig)
    saveConfig();

  this->sendPoll(1);
}

/**
 * ArtInput
 *
 * --------------------------------------------------------------------------------------
 * Entity      | Direction          | Action
 * --------------------------------------------------------------------------------------
 *  Controller | Receive            | No Action
 *             | Unicast Transmit   | Controller transmits to a specific node IP address.
 *             | Directed Broadcast | Not Allowed.
 *  Node       | Receive            | Reply with ArtPollReply.
 *             | Unicast Transmit   | Not Allowed.
 *             | Broadcast          | Not Allowed.
 *  Media      | Receive            | Reply with ArtPollReply.
 *  Server     | Unicast Transmit   | Not Allowed.
 *             | Broadcast          | Not Allowed.
 * ---------------------------------------------------------------------------------------
 *
 * A Controller or monitoring device on the network can enable or disable individual
 * DMX512 inputs on any of the network nodes. This allows the Controller to directly
 * control network traffic and ensures that unused inputs are disabled and therefore not
 * wasting bandwidth.
 *
 * All nodes power on with all inputs enabled.
 *
 * Caution should be exercised when implementing this function in the controller. Keep in
 * mind that some network traffic may be operating on a node to node basis. 
 */

void ArtNet::processInput(byte ip[4], word port, const char *data, word len)
{
  artnetinput_t *input = (artnetinput_t *)data;
  unsigned char i;
  
  for(i = 0; i < MAX_PORTS; i++)
  {
    cfg.inputEnable[i] = input->input[i];
  }

  saveConfig();
}

/**
 * ArtOutput / ArtDmx
 *
 * --------------------------------------------------------------------------------------
 * Entity      | Direction          | Action
 * --------------------------------------------------------------------------------------
 *  Controller | Receive            | Application Specific
 *             | Unicast Transmit   | Yes
 *             | Broadcast          | No
 *  Node       | Receive            | Application Specific
 *             | Unicast Transmit   | Yes
 *             | Broadcast          | No
 *  Media      | Receive            | Application Specific
 *  Server     | Unicast Transmit   | Yes
 *             | Broadcast          | No
 * ---------------------------------------------------------------------------------------
 *
 * ArtDmx is the data packet used to transfer DMX512 data. The format is identical for
 * Node to Controller, Node to Node and Controller to Node.
 * The Data is output through the DMX O/P port corresponding to the Universe setting. In
 * the absence of received ArtDmx packets, each DMX O/P port re-transmits the same
 * frame continuously.
 * 
 * The first complete DMX frame received at each input port is placed in an ArtDmx packet
 * as above and transmitted as an ArtDmx packet containing the relevant Universe
 * parameter. Each subsequent DMX frame containing new data (different length or
 * different contents) is also transmitted as an ArtDmx packet.
 * Nodes do not transmit ArtDmx for DMX512 inputs that have not received data since
 * power on.
 *
 * However, an input that is active but not changing, will re-transmit the last valid ArtDmx
 * packet at approximately 4-second intervals. (Note. In order to converge the needs of ArtNet
 * and sACN it is recommended that Art-Net devices actually use a re-transmit time of
 * 800mS to 1000mS).
 * A DMX input that fails will not continue to transmit ArtDmx data.
 *
 * *** Unicast Subscription:
 *
 * ArtDmx packets must be unicast to subscribers of the specific universe contained in the
 * ArtDmx packet.
 * 
 * The transmitting device must regularly ArtPoll the network to detect any change in
 * devices which are subscribed. Nodes that are subscribed will list the subscription
 * universe in the ArtPollReply. Subscribed means any universes listed in either the Swin or
 * Swout array.
 * 
 * If there are no subscribers to a universe that the transmitter wishes to send, then the
 * ArtDmx must not be broadcast. If the number of universe subscribers exceeds 40 for a
 * given universe, the transmitting device may broadcast.
 *
 */
void ArtNet::processOutput(byte ip[4], word port, const char *data, word len)
{
  artnetoutput_t *out = (artnetoutput_t *)data;
  
  unsigned char i;
        
  uint16_t universe = out->subUni | (out->net << 8);
  uint16_t length = out->length | (out->lengthH << 8);

  // This needs to be changed, makes no sense to call the same callback for all ports
  // each port calls a call back or, only callback on the ports we are processing
  for (i = 0; i < this->ports; i++) {
    if (cfg.inUniverse[i] == universe
        && cfg.inputEnable[i])
    {
      this->callback(i, (const char*)&out->data, length);
    }
  }
}
