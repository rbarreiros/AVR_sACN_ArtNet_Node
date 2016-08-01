#ifndef __ARTNETDEFS_H__
#define __ARTNETDEFS_H__

/* Protocol defines
 * this should only be changes if protocol changes
 */

#define ARTNET_VERSION 14

#define UDP_PORT_ARTNET       6454  /* (0x1936) */

#define NODE_NAME "AVR Art-Net Node"

typedef enum ArtNetPortTypeTag
{
  ARTNET_OFF,
  ARTNET_IN,
  ARTNET_OUT
} ArtNetPortType;

typedef enum
{
  PRIMARY = 0,
  SECONDARY,
  DHCP,
  CUSTOM
} IPConfiguration;

typedef enum ArtNetStatusTag
{
  ARTNET_STATUS_DEBUG              = 0x0000,
  ARTNET_STATUS_POWER_OK           = 0x0001,
  ARTNET_STATUS_POWER_FAIL         = 0x0002,
  ARTNET_STATUS_READ_FAIL          = 0x0003,
  ARTNET_STATUS_PARSE_FAIL         = 0x0004,
  ARTNET_STATUS_WRITE_FAIL         = 0x0005,
  ARTNET_STATUS_SHORT_NAME_SUCCESS = 0x0006,
  ARTNET_STATUS_LONG_NAME_SUCCESS  = 0x0007,
  ARTNET_STATUS_DMX_ERRORS         = 0x0008,
  ARTNET_STATUS_WRITE_BUFFER_FULL  = 0x0009,
  ARTNET_STATUS_READ_BUFFER_FULL   = 0x000a,
  ARTNET_STATUS_UNIVERSE_CONFLICT  = 0x000b,
  ARTNET_STATUS_CONFIGURATION_FAIL = 0x000c,
  ARTNET_STATUS_DMX_OUTPUT_SHORT   = 0x000d,
  ARTNET_STATUS_FIRMWARE_FAIL      = 0x000e,
  ARTNET_STATIS_USER_FAIL          = 0x000f
} ArtNetStatus_t;

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

// Address commands

typedef enum
{
  AC_NONE = 0,      // No action
  AC_CANCEL_MERGE,  // If Node is currently in merge mode, cancel merge mode upon receipt of next ArtDmx packet.
  AC_LED_NORMAL,    // The front panel indicators of the Node operate normally.
  AC_LED_MUTE,      // The front panel indicators of the Node are disabled and switched off.
  AC_LED_LOCATE,    // Rapid flashing of the Node’s front panel indicators. It is intended as an outlet identifier for large installations.
  AC_RESET_RX_FLAGS,// Resets the Node’s Sip, Text, Test and data error flags. If an output short is being flagged, forces the test to re-run.

  AC_MERGE_LTP0 = 0x10,  // Set DMX Port 0 to Merge in LTP mode.
  AC_MERGE_LTP1 = 0x11,
  AC_MERGE_LTP2 = 0x12,
  AC_MERGE_LTP3 = 0x13,

  AC_MERGE_HTP0 = 0x50,  // Set DMX Port 0 to Merge in HTP (default) mode.
  AC_MERGE_HTP1 = 0x51,
  AC_MERGE_HTP2 = 0x52,
  AC_MERGE_HTP3 = 0x53,

  AC_CLEAR_OP0  = 0x90,  // Clear DMX Output buffer for Port 0
  AC_CLEAR_OP1  = 0x91,
  AC_CLEAR_OP2  = 0x92,
  AC_CLEAR_OP3  = 0x93
} ArtNetAddressCmd;

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
} ArtNetNodeStyle;

/**************************************************************************
 * Types
 **************************************************************************/

typedef enum ArtNetOpCodeTag
{
  ARTNET_OP_POLL                = 0x2000,
  ARTNET_OP_POLL_REPLY          = 0x2100,
  ARTNET_OP_DIAG_DATA           = 0x2300,
  ARTNET_OP_OUTPUT              = 0x5000,
  ARTNET_OP_ADDRESS             = 0x6000,
  ARTNET_OP_INPUT               = 0x7000,
  ARTNET_OP_TOD_REQUEST         = 0x8000,
  ARTNET_OP_TOD_DATA            = 0x8100,
  ARTNET_OP_TOD_CONTROL         = 0x8200,
  ARTNET_OP_RDM                 = 0x8300,
  ARTNET_OP_RDM_SUB             = 0x8400,
  ARTNET_OP_VIDEO_SETUP         = 0xa010,
  ARTNET_OP_VIDEO_PALETTE       = 0xa020,
  ARTNET_OP_VIDEO_DATA          = 0xa040,
  ARTNET_OP_MAC_MASTER          = 0xf000,
  ARTNET_OP_MAC_SLAVE           = 0xf100,
  ARTNET_OP_FIRMWARE_MASTER     = 0xf200,
  ARTNET_OP_FIRMWARE_REPLY      = 0xf300,
  ARTNET_OP_IP_PROG             = 0xf800,
  ARTNET_OP_IP_PROG_REPLY       = 0xf900,
  ARTNET_OP_MEDIA               = 0x9000,
  ARTNET_OP_MEDIA_PATCH         = 0x9100,
  ARTNET_OP_MEDIA_CONTROL       = 0x9200,
  ARTNET_OP_MEDIA_CONTROL_REPLY = 0x9300,
  ARTNET_OP_TIMECODE            = 0x9700,
} ArtNetOpCode;

typedef struct
{
  uint8_t id[8];
  ArtNetOpCode opcode;
  uint8_t verH;
  uint8_t ver;
} artnetheader_t;

typedef enum ArtNetPriorityTag
{
  ARTNET_DIAGNOSTIC_LOW      = 0x10,
  ARTNET_DIAGNOSTIC_MED      = 0x40,
  ARTNET_DIAGNOSTIC_HIGH     = 0x80,
  ARTNET_DIAGNOSTIC_CRITICAL = 0xe0,
  ARTNET_DIAGNOSTIC_VOLATILE = 0xff
} ArtNetPriority;

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
  artnetheader_t header;
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
  uint8_t  shortName[18];
  uint8_t  longName[64];
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
  artnetheader_t header;
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
  artnetheader_t header;
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
  artnetheader_t header;
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

typedef struct
{
  artnetheader_t header;
  uint16_t filler;
  uint8_t  numPortsH;
  uint8_t  numPorts;
  uint8_t  input[4];
} artnetinput_t;

typedef struct
{
  artnetheader_t header;
  uint8_t  sequence;
  uint8_t  physical;
  uint8_t  subUni;
  uint8_t  net;
  uint8_t  lengthH;
  uint8_t  length;
  uint8_t  data; // Variable size
} artnetoutput_t;


#endif
