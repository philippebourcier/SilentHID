#ifndef TYPES_H
#define TYPES_H

struct HttpRequest {
  char method[8];
  char path[64];
  char websocketKey[64];
  char authorization[128];
  bool isWebSocketUpgrade;
};

struct WsFrame {
  uint8_t fin;
  uint8_t opcode;
  uint8_t masked;
  uint8_t mask[4];
  size_t payloadLen;
  uint8_t* payload;
};

#endif