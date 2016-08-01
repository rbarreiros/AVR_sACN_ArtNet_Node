#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <ArtNet.h>
#include <stdint.h>

#define EEPROM_CONFIG 253

struct Config {
  IPConfiguration iptype;
  uint8_t ip[4];
  uint8_t gateway[4];
  uint8_t valid;
};



class Application
{
public:

  Application();
  void begin(void);
  void loop(void);

private:
  bool loadConfig(void);
  void saveConfig(void);
  void setDefaults(void);
  bool configureNetwork(void);

  ArtNet *artnet;
  struct Config config;
};

#endif
