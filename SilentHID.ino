/*
 * W55RP20 HTTPS WebSocket HID Server
 * TLS + WebSocket + USB HID (Mouse/Keyboard)
 */

#include <Arduino.h>
#include <SPI.h>
#include <W55RP20_Ethernet3.h>
#include <bearssl/bearssl.h>
#include "Adafruit_TinyUSB.h"
#include "config.h"
#include "certs.h"
#include "types.h"

// ============================================================================
// USB HID Setup
// ============================================================================
uint8_t const desc_hid_report[] = {
  TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(1)),
  TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(2))
};
Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_NONE, 2, false);

// ============================================================================
// BearSSL Polyfill
// ============================================================================
extern "C" uint32_t __picoRand(void) {
  return rp2040.hwrand32();
}

// ============================================================================
// Globals
// ============================================================================
static const char* WS_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
static const char b64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

byte mac[] = NET_MAC;
IPAddress ip = NET_IP;

EthernetServer server(NET_PORT);
EthernetClient* currentClient = nullptr;

unsigned char iobuf[BR_SSL_BUFSIZE_BIDI];
br_ssl_server_context sc;
br_x509_certificate chain[1];
br_rsa_private_key sk;
br_skey_decoder_context dc;

static uint8_t mouse_buttons = 0;
static unsigned long lastHidReport = 0;

// ============================================================================
// Utility Functions
// ============================================================================
int base64_encode(char* output, const uint8_t* input, int inputLen) {
  int i = 0, j = 0, encLen = 0;
  uint8_t a3[3], a4[4];
  while (inputLen--) {
    a3[i++] = *(input++);
    if (i == 3) {
      a4[0] = (a3[0] & 0xfc) >> 2;
      a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
      a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
      a4[3] = a3[2] & 0x3f;
      for (i = 0; i < 4; i++) output[encLen++] = b64_alphabet[a4[i]];
      i = 0;
    }
  }
  if (i) {
    for (j = i; j < 3; j++) a3[j] = '\0';
    a4[0] = (a3[0] & 0xfc) >> 2;
    a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
    a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
    a4[3] = a3[2] & 0x3f;
    for (j = 0; j < i + 1; j++) output[encLen++] = b64_alphabet[a4[j]];
    while (i++ < 3) output[encLen++] = '=';
  }
  output[encLen] = '\0';
  return encLen;
}

// ============================================================================
// TLS Functions
// ============================================================================
bool decode_key(const unsigned char* buf, size_t len, br_rsa_private_key* key) {
  br_skey_decoder_init(&dc);
  br_skey_decoder_push(&dc, buf, len);
  if (br_skey_decoder_last_error(&dc) != 0) return false;
  if (br_skey_decoder_key_type(&dc) != BR_KEYTYPE_RSA) return false;
  *key = *br_skey_decoder_get_rsa(&dc);
  return true;
}

bool initTLS() {
  if (!decode_key(server_key, sizeof(server_key), &sk)) return false;
  chain[0].data = (unsigned char*)server_cert;
  chain[0].data_len = sizeof(server_cert);
  br_ssl_server_init_full_rsa(&sc, chain, 1, &sk);
  br_ssl_engine_set_buffer(&sc.eng, iobuf, sizeof(iobuf), 1);
  br_ssl_engine_inject_entropy(&sc.eng, (void*)&rp2040, sizeof(rp2040));
  return true;
}

void run_ssl_engine(EthernetClient* c) {
  if (!c) return;
  if (c->available()) {
    size_t len;
    unsigned char* buf = br_ssl_engine_recvrec_buf(&sc.eng, &len);
    if (len > 0) {
      int r = c->read(buf, len);
      if (r > 0) br_ssl_engine_recvrec_ack(&sc.eng, r);
    }
  }
  size_t len;
  unsigned char* buf = br_ssl_engine_sendrec_buf(&sc.eng, &len);
  if (len > 0) {
    int w = c->write(buf, len);
    if (w > 0) {
      br_ssl_engine_sendrec_ack(&sc.eng, w);
      c->flush();
    }
  }
}

int tlsRead(uint8_t* buf, size_t len, unsigned long timeout) {
  if (!currentClient || !currentClient->connected()) return 0;
  unsigned long start = millis();
  size_t totalRead = 0;
  while (totalRead < len) {
    run_ssl_engine(currentClient);
    size_t appLen;
    uint8_t* appBuf = br_ssl_engine_recvapp_buf(&sc.eng, &appLen);
    if (appBuf != NULL && appLen > 0) {
      size_t toCopy = (len - totalRead) < appLen ? (len - totalRead) : appLen;
      memcpy(buf + totalRead, appBuf, toCopy);
      br_ssl_engine_recvapp_ack(&sc.eng, toCopy);
      totalRead += toCopy;
    }
    if (millis() - start > timeout) return totalRead;
    if (!currentClient->connected()) return -1;
  }
  return totalRead;
}

void tlsWrite(const uint8_t* data, size_t len) {
  if (!currentClient || !currentClient->connected()) return;
  size_t written = 0;
  while (written < len && currentClient->connected()) {
    run_ssl_engine(currentClient);
    size_t alen;
    unsigned char* buf = br_ssl_engine_sendapp_buf(&sc.eng, &alen);
    if (alen > 0) {
      size_t chunk = (len - written) < alen ? (len - written) : alen;
      memcpy(buf, data + written, chunk);
      br_ssl_engine_sendapp_ack(&sc.eng, chunk);
      written += chunk;
    }
  }
  br_ssl_engine_flush(&sc.eng, 0);
  run_ssl_engine(currentClient);
}

// ============================================================================
// HTTP/WebSocket Functions
// ============================================================================
bool checkBasicAuth(const char* authHeader) {
  if (strncmp(authHeader, "Basic ", 6) != 0) return false;
  char creds[64];
  snprintf(creds, sizeof(creds), "%s:%s", WS_USERNAME, WS_PASSWORD);
  char expected[128];
  base64_encode(expected, (const uint8_t*)creds, strlen(creds));
  return strcmp(authHeader + 6, expected) == 0;
}

void computeWebSocketAccept(const char* wsKey, char* acceptKey) {
  char combined[128];
  snprintf(combined, sizeof(combined), "%s%s", wsKey, WS_GUID);
  uint8_t hash[20];
  br_sha1_context ctx;
  br_sha1_init(&ctx);
  br_sha1_update(&ctx, combined, strlen(combined));
  br_sha1_out(&ctx, hash);
  base64_encode(acceptKey, hash, 20);
}

bool readWsFrame(WsFrame& frame, uint8_t* buffer, size_t bufferSize) {
  uint8_t header[2];
  if (tlsRead(header, 2, WS_FRAME_TIMEOUT_MS) != 2) return false;
  frame.fin = (header[0] >> 7) & 1;
  frame.opcode = header[0] & 0x0F;
  frame.masked = (header[1] >> 7) & 1;
  frame.payloadLen = header[1] & 0x7F;
  if (frame.payloadLen == 126) {
    uint8_t ext[2];
    if (tlsRead(ext, 2, WS_FRAME_TIMEOUT_MS) != 2) return false;
    frame.payloadLen = ((uint16_t)ext[0] << 8) | ext[1];
  } else if (frame.payloadLen == 127) {
    uint8_t ext[8];
    if (tlsRead(ext, 8, WS_FRAME_TIMEOUT_MS) != 8) return false;
    return false;
  }
  if (frame.masked && tlsRead(frame.mask, 4, WS_FRAME_TIMEOUT_MS) != 4) return false;
  if (frame.payloadLen > bufferSize) return false;
  size_t received = 0;
  while (received < frame.payloadLen) {
    int r = tlsRead(buffer + received, frame.payloadLen - received, WS_FRAME_TIMEOUT_MS);
    if (r <= 0) return false;
    received += r;
  }
  if (frame.masked) {
    for (size_t i = 0; i < frame.payloadLen; i++) buffer[i] ^= frame.mask[i % 4];
  }
  frame.payload = buffer;
  return true;
}

void sendWsFrame(uint8_t opcode, uint8_t* data, size_t len) {
  uint8_t head[10];
  int idx = 0;
  head[idx++] = 0x80 | opcode;
  if (len < 126) head[idx++] = len;
  else {
    head[idx++] = 126;
    head[idx++] = len >> 8;
    head[idx++] = len & 0xFF;
  }
  tlsWrite(head, idx);
  if (len > 0) tlsWrite(data, len);
}

// ============================================================================
// HID Functions
// ============================================================================
bool hid_wait_ready() {
  unsigned long start = millis();
  while (!usb_hid.ready()) {
    if (millis() - start > HID_READY_TIMEOUT_MS) return false;
    delay(1);
  }
  unsigned long elapsed = millis() - lastHidReport;
  if (elapsed < HID_MIN_INTERVAL_MS) delay(HID_MIN_INTERVAL_MS - elapsed);
  return true;
}

void hid_mouse_move(int8_t x, int8_t y, int8_t scroll) {
  if (!hid_wait_ready()) return;
  usb_hid.mouseReport(2, mouse_buttons, x, y, scroll, 0);
  lastHidReport = millis();
}

void hid_mouse_click(uint8_t button) {
  if (!hid_wait_ready()) return;
  usb_hid.mouseReport(2, button, 0, 0, 0, 0);
  lastHidReport = millis();
  delay(50);
  if (!hid_wait_ready()) return;
  usb_hid.mouseReport(2, 0, 0, 0, 0, 0);
  lastHidReport = millis();
}

void hid_mouse_press(uint8_t button) {
  mouse_buttons |= button;
  if (!hid_wait_ready()) return;
  usb_hid.mouseReport(2, mouse_buttons, 0, 0, 0, 0);
  lastHidReport = millis();
}

void hid_mouse_release(uint8_t button) {
  mouse_buttons &= ~button;
  if (!hid_wait_ready()) return;
  usb_hid.mouseReport(2, mouse_buttons, 0, 0, 0, 0);
  lastHidReport = millis();
}

void hid_keyboard_press(uint8_t keycode) {
  if (!hid_wait_ready()) return;
  uint8_t keycodes[6] = { keycode, 0, 0, 0, 0, 0 };
  usb_hid.keyboardReport(1, 0, keycodes);
  lastHidReport = millis();
}

void hid_keyboard_release() {
  if (!hid_wait_ready()) return;
  usb_hid.keyboardReport(1, 0, NULL);
  lastHidReport = millis();
}

void hid_keyboard_type(const char* str) {
  while (*str) {
    uint8_t keycode = 0, modifier = 0;
    char c = *str++;
    if (c >= 'a' && c <= 'z') keycode = 4 + (c - 'a');
    else if (c >= 'A' && c <= 'Z') { keycode = 4 + (c - 'A'); modifier = KEYBOARD_MODIFIER_LEFTSHIFT; }
    else if (c >= '1' && c <= '9') keycode = 30 + (c - '1');
    else if (c == '0') keycode = 39;
    else if (c == ' ') keycode = 44;
    else if (c == '\n') keycode = 40;
    if (keycode) {
      if (!hid_wait_ready()) return;
      uint8_t keycodes[6] = { keycode, 0, 0, 0, 0, 0 };
      usb_hid.keyboardReport(1, modifier, keycodes);
      lastHidReport = millis();
      delay(10);
      if (!hid_wait_ready()) return;
      usb_hid.keyboardReport(1, 0, NULL);
      lastHidReport = millis();
      delay(10);
    }
  }
}

void hid_reset() {
  hid_keyboard_release();
  mouse_buttons = 0;
  if (usb_hid.ready()) usb_hid.mouseReport(2, 0, 0, 0, 0, 0);
}

// ============================================================================
// WebSocket Session Handler
// ============================================================================
void handleWebSocketSession() {
  DEBUG_PRINTLN("WS Session Active");
  uint8_t buf[256];
  WsFrame frame;
  unsigned long lastMouseMove = 0;

  while (currentClient && currentClient->connected()) {
    run_ssl_engine(currentClient);
    
    if (readWsFrame(frame, buf, sizeof(buf))) {
      if (frame.opcode == 0x1) {  // TEXT
        buf[frame.payloadLen] = 0;
        char* cmd = (char*)buf;
        
        if (strncmp(cmd, "MM:", 3) == 0) {
          unsigned long now = millis();
          if (now - lastMouseMove >= MOUSE_THROTTLE_MS) {
            int x = 0, y = 0;
            if (sscanf(cmd + 3, "%d,%d", &x, &y) == 2) {
              hid_mouse_move(x, y, 0);
              lastMouseMove = now;
            }
          }
          sendWsFrame(0x1, (uint8_t*)"OK", 2);
        }
        else if (strncmp(cmd, "MC:", 3) == 0) {
          uint8_t b = (cmd[3] == 'l') ? MOUSE_BUTTON_LEFT : (cmd[3] == 'r') ? MOUSE_BUTTON_RIGHT : MOUSE_BUTTON_MIDDLE;
          hid_mouse_click(b);
          sendWsFrame(0x1, (uint8_t*)"OK", 2);
        }
        else if (strncmp(cmd, "MP:", 3) == 0) {
          uint8_t b = (cmd[3] == 'l') ? MOUSE_BUTTON_LEFT : (cmd[3] == 'r') ? MOUSE_BUTTON_RIGHT : MOUSE_BUTTON_MIDDLE;
          hid_mouse_press(b);
          sendWsFrame(0x1, (uint8_t*)"OK", 2);
        }
        else if (strncmp(cmd, "MR:", 3) == 0) {
          uint8_t b = (cmd[3] == 'l') ? MOUSE_BUTTON_LEFT : (cmd[3] == 'r') ? MOUSE_BUTTON_RIGHT : MOUSE_BUTTON_MIDDLE;
          hid_mouse_release(b);
          sendWsFrame(0x1, (uint8_t*)"OK", 2);
        }
        else if (strncmp(cmd, "MS:", 3) == 0) {
          hid_mouse_move(0, 0, atoi(cmd + 3));
          sendWsFrame(0x1, (uint8_t*)"OK", 2);
        }
        else if (strncmp(cmd, "KT:", 3) == 0) {
          hid_keyboard_type(cmd + 3);
          sendWsFrame(0x1, (uint8_t*)"OK", 2);
        }
        else if (strncmp(cmd, "KP:", 3) == 0) {
          hid_keyboard_press((uint8_t)atoi(cmd + 3));
          sendWsFrame(0x1, (uint8_t*)"OK", 2);
        }
        else if (strcmp(cmd, "KRA") == 0) {
          hid_keyboard_release();
          sendWsFrame(0x1, (uint8_t*)"OK", 2);
        }
        else {
          sendWsFrame(0x1, (uint8_t*)"ERR", 3);
        }
      }
      else if (frame.opcode == 0x9) sendWsFrame(0xA, frame.payload, frame.payloadLen);  // PING->PONG
      else if (frame.opcode == 0x8) break;  // CLOSE
    }
    delay(1);
  }
  
  hid_reset();
  DEBUG_PRINTLN("WS Session Ended");
}

// ============================================================================
// Setup & Loop
// ============================================================================
void setup() {
  usb_hid.begin();
  Serial.begin(115200);
  delay(2000);
  
  Ethernet.begin(mac, ip);
  Serial.print("HTTPS HID Server: https://");
  Serial.println(Ethernet.localIP());
  
  server.begin();
  
  if (!initTLS()) {
    Serial.println("TLS Init Failed!");
    while (1) delay(1000);
  }
  
  Serial.println("Ready.");
}

void loop() {
  EthernetClient client = server.available();
  if (!client) return;
  
  DEBUG_PRINTLN("New Connection");
  currentClient = &client;
  br_ssl_server_reset(&sc);
  
  bool handled = false;
  unsigned long timeout = millis();
  
  while (client.connected()) {
    run_ssl_engine(&client);
    
    size_t len;
    uint8_t* buf = br_ssl_engine_recvapp_buf(&sc.eng, &len);
    
    if (len > 0) {
      char reqBuf[512];
      int copyLen = (len < sizeof(reqBuf) - 1) ? len : sizeof(reqBuf) - 1;
      memcpy(reqBuf, buf, copyLen);
      reqBuf[copyLen] = 0;
      br_ssl_engine_recvapp_ack(&sc.eng, len);
      
      if (strstr(reqBuf, "GET / HTTP")) {
        bool isWsUpgrade = strstr(reqBuf, "Upgrade: websocket") != NULL;
        char* keyStart = strstr(reqBuf, "Sec-WebSocket-Key: ");
        char* authStart = strstr(reqBuf, "Authorization: ");
        char* authEnd = authStart ? strchr(authStart + 15, '\r') : NULL;
        if (authEnd) *authEnd = '\0';
        
        if (!authStart || !checkBasicAuth(authStart + 15)) {
          if (authEnd) *authEnd = '\r';
          const char* r = "HTTP/1.1 401 Unauthorized\r\nWWW-Authenticate: Basic realm=\"HID\"\r\nConnection: close\r\n\r\n";
          tlsWrite((uint8_t*)r, strlen(r));
          handled = true;
        }
        else if (isWsUpgrade && keyStart) {
          if (authEnd) *authEnd = '\r';
          keyStart += 19;
          char* keyEnd = strchr(keyStart, '\r');
          if (keyEnd) {
            *keyEnd = 0;
            char acceptKey[64];
            computeWebSocketAccept(keyStart, acceptKey);
            char resp[256];
            snprintf(resp, sizeof(resp),
              "HTTP/1.1 101 Switching Protocols\r\n"
              "Upgrade: websocket\r\n"
              "Connection: Upgrade\r\n"
              "Sec-WebSocket-Accept: %s\r\n\r\n", acceptKey);
            tlsWrite((uint8_t*)resp, strlen(resp));
            handleWebSocketSession();
            handled = true;
            break;
          }
        }
        else {
          if (authEnd) *authEnd = '\r';
          DEBUG_PRINTLN("Serving HTML...");
          
          // Calculate content length
          size_t htmlLen = strlen_P(html_page);
          
          // Send headers
          char headers[128];
          snprintf(headers, sizeof(headers),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %d\r\n"
            "Connection: close\r\n\r\n", htmlLen);
          tlsWrite((uint8_t*)headers, strlen(headers));
          
          // Send HTML in chunks (PROGMEM)
          const char* ptr = html_page;
          char chunk[256];
          size_t remaining = htmlLen;
          while (remaining > 0) {
            size_t chunkSize = remaining > sizeof(chunk) ? sizeof(chunk) : remaining;
            memcpy_P(chunk, ptr, chunkSize);
            tlsWrite((uint8_t*)chunk, chunkSize);
            ptr += chunkSize;
            remaining -= chunkSize;
          }
          
          handled = true;

        //   if (authEnd) *authEnd = '\r';
        //   const char* r = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nHID Server OK";
        //   tlsWrite((uint8_t*)r, strlen(r));
        //   handled = true;
        }
      }
    }
    
    if (handled) {
      unsigned long flush = millis();
      while (millis() - flush < TLS_FLUSH_TIMEOUT_MS) run_ssl_engine(&client);
      br_ssl_engine_close(&sc.eng);
      flush = millis();
      while (millis() - flush < TLS_FLUSH_TIMEOUT_MS) {
        run_ssl_engine(&client);
        if (br_ssl_engine_current_state(&sc.eng) == BR_SSL_CLOSED) break;
      }
      break;
    }
    
    if (millis() - timeout > CLIENT_TIMEOUT_MS) {
      br_ssl_engine_close(&sc.eng);
      break;
    }
  }
  
  client.stop();
  currentClient = nullptr;
  DEBUG_PRINTLN("Done.");
}