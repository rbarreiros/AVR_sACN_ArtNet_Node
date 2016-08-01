#ifndef __BROWSER_PAGES_H__
#define __BROWSER_PAGES_H__

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

#endif
