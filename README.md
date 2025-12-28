
<p align="center">
  <img src="https://github.com/philippebourcier/SilentHID/blob/main/silenthid-v1.svg?raw=true" alt="SilentHID" width="300">
</p>

# SilentHID

HID (Keyboard & Mouse) over Secure WebSocket for robotic remote control of PCs.

SilentHID enables humanoid robots or remote systems to control a PC's keyboard and mouse without physical interaction. The device appears as a standard USB HID device to the target computer while receiving commands over a secure TLS WebSocket connection.

This solution is part of a Vision Based GUI Agent pipeline, enabling humanoid robots to interact with standard PC/Mac/Mobile interfaces through visual perception and physical-free HID control. A separate solution is in charge of screen capture (with passthrough), analysis and translating a list of actions, like  "Add a new row in this excel file and type ABC123", into SilentHID actions. This way, for rapid deployment and trial, the human can replace the machine, and vice versa without the need for complex integration with the customer software stack and/or ERP APIs.

## Features

- **Secure WebSocket (WSS)** - TLS 1.2 encrypted communication using BearSSL
- **USB HID Emulation** - Appears as native keyboard and mouse to the host PC
- **Basic Authentication** - Password-protected access
- **Built-in Web Interface** - Test control panel served directly from the device
- **Low Latency** - Optimized for real-time robotic control
- **No Drivers Required** - Standard USB HID, works on any OS or machining equipment (CNC machine, etc)

## Hardware Requirements

- **WIZnet W55RP20-EVB-Pico** (RP2040 + W5500 Ethernet)
- USB cable for HID connection to target PC
- Ethernet connection to robot/control network
- It is possible also to imagine a SPE connector near each PC where a humanoid robot would plug itself to take control of the PC by directly sending commands

## Quick Start

### 1. Configure

Edit `config.h`:

```cpp
#define NET_IP IPAddress(10, 200, 200, 200)
#define NET_PORT 443
#define WS_USERNAME "admin"
#define WS_PASSWORD "secret"
```

You only need to change one line in setup() to enable DHCP:
- Static IP: cppEthernet.begin(mac, ip);
- DHCP: cppEthernet.begin(mac);

### 2. Generate TLS Certificate

```bash
# Generate private key and self-signed certificate
openssl req -x509 -newkey rsa:2048 -keyout key.pem -out cert.pem -days 3650 -nodes

# Convert to DER format
openssl x509 -in cert.pem -outform DER -out cert.der
openssl pkcs8 -topk8 -nocrypt -in key.pem -outform DER -out key.der

# Convert to C arrays (copy output to certs.h)
xxd -i cert.der
xxd -i key.der
```

### 3. Flash

1. Open the .INO file in Arduino IDE
2. Connect the w55rp20-evb-pico via USB and set speed to 115200 in the Serial Monitor
3. Select board: **WIZnet W55RP20-EVB-Pico**
4. Select USB Stack: **Adafruit TinyUSB**
5. While holding down the BOOTSEL button, press the Reset button briefly once
6. Click Upload in Arduino IDE

### 4. Connect

1. Connect USB to target PC (HID device)
2. Connect Ethernet to your network
3. Open `https://IP_address_you_set/` (10.200.200.200 (/24) by default) in a browser
4. Accept the self-signed certificate
5. Login with your credentials (admin/secret by default)

## Web Interface

The built-in control panel provides:

- **Keyboard** - Text input, function keys, special keys, arrow keys, shortcuts
- **Mouse** - Trackpad, left/middle/right buttons, click, double-click, scroll
- **Raw Commands** - Direct command input for automation

## WebSocket Protocol

Commands are sent as plain text over the WebSocket connection:

### Relative Mouse Commands

| Command | Format | Description |
|---------|--------|-------------|
| Mouse Move | `MM:x,y` | Relative mouse movement (-127 to 127) |
| Mouse Click | `MC:l\|r\|m` | Click left, right, or middle button |
| Mouse Press | `MP:l\|r\|m` | Press and hold button |
| Mouse Release | `MR:l\|r\|m` | Release button |
| Mouse Scroll | `MS:n` | Scroll wheel (negative = down) |

### Absolute Mouse Commands

| Command | Format | Description |
|---------|--------|-------------|
| Absolute Move | `MA:x,y` | Move cursor to pixel position |
| Ratio Move | `MF:rx,ry` | Move cursor to ratio position (0.0-1.0) |
| Click At | `CA:x,y,b` | Click at pixel position (b = l/r/m) |
| Click At Ratio | `CF:rx,ry,b` | Click at ratio position (b = l/r/m) |

### Screen Resolution

| Command | Format | Description |
|---------|--------|-------------|
| Set Resolution | `SR:w,h` | Set screen resolution (for absolute positioning) |
| Get Resolution | `GR` | Get current screen resolution |

### Keyboard Commands

| Command | Format | Description |
|---------|--------|-------------|
| Keyboard Type | `KT:text` | Type a string |
| Keyboard Press | `KP:keycode` | Press a key (HID keycode) |
| Keyboard Release | `KRA` | Release all keys |

### HID Keycodes

| Key | Code | Key | Code | Key | Code |
|-----|------|-----|------|-----|------|
| A-Z | 4-29 | 1-9 | 30-38 | 0 | 39 |
| Enter | 40 | Escape | 41 | Backspace | 42 |
| Tab | 43 | Space | 44 | F1-F12 | 58-69 |
| Insert | 73 | Home | 74 | Page Up | 75 |
| Delete | 76 | End | 77 | Page Down | 78 |
| Right | 79 | Left | 80 | Down | 81 |
| Up | 82 | | | | |

### System Commands

| Command | Format | Description |
|---------|--------|-------------|
| Reset | `RST` | Reset all HID states |
| Ping | `PNG` | Keep-alive ping (returns `PON`) |

All commands return `OK` on success or `ERR:reason` on failure.
Set Resolution returns `OK:w,h` with the current resolution.
Get Resolution returns `w,h`.

## Integration Example

### Python

```python
import asyncio
import websockets
import ssl

async def control():
    ssl_ctx = ssl.create_default_context()
    ssl_ctx.check_hostname = False
    ssl_ctx.verify_mode = ssl.CERT_NONE
    
    uri = "wss://admin:secret@10.200.200.200/"
    
    async with websockets.connect(uri, ssl=ssl_ctx) as ws:
        # Type "Hello World"
        await ws.send("KT:Hello World")
        print(await ws.recv())
        
        # Move mouse
        await ws.send("MM:100,50")
        print(await ws.recv())
        
        # Click
        await ws.send("MC:l")
        print(await ws.recv())

asyncio.run(control())
```

### JavaScript

```javascript
const ws = new WebSocket('wss://admin:secret@10.200.200.200/');

ws.onopen = () => {
    ws.send('KT:Hello World');
    ws.send('MM:100,50');
    ws.send('MC:l');
};

ws.onmessage = (e) => console.log('Response:', e.data);
```

## Architecture

```
┌─────────────────┐     WSS/TLS     ┌─────────────────┐     USB HID     ┌─────────────────┐
│                 │◄───────────────►│                 │◄───────────────►│                 │
│  Robot / Client │     Ethernet    │    SilentHID    │                 │    Target PC    │
│                 │                 │  (W55RP20-EVB)  │                 │                 │
└─────────────────┘                 └─────────────────┘                 └─────────────────┘
```

## Project Structure

These are the files needed in your Arduino Sketch folder:
```
SilentHID/
├── SilentHID.ino    # Main application
├── config.h         # Configuration (network, auth, timing)
├── certs.h          # TLS certificate and private key
└── types.h          # Struct definitions
```

## Security Considerations

- **Change default credentials** before deployment
- **Generate unique TLS certificates** for each device
- **Network isolation** - Deploy on a dedicated robot control network
- **Self-signed certificates** - Add to your robot's trusted store or use a proper CA

## Troubleshooting

| Issue | Solution |
|-------|----------|
| Connection refused | Check IP address and firewall settings |
| Certificate error | Accept self-signed cert or add to trusted store |
| HID not recognized | Ensure USB Stack is set to "Adafruit TinyUSB" in Arduino IDE |
| Mouse moves erratically | Reduce sensitivity or increase throttle interval |
| Keyboard not typing | Check HID keycodes, ensure KRA sent after KP |

## Tutorial

In order to setup your dev environment, do not hesitate to follow this tutorial :
[WIZnet W55RP20 Arduino Tutorial](https://maker.wiznet.io/mason/projects/getting-started-with-ethernet3-on-w55rp20-using-arduino/)

## Dependencies

- [arduino-pico](https://github.com/earlephilhower/arduino-pico) - RP2040 Arduino core
- [Adafruit TinyUSB](https://github.com/adafruit/Adafruit_TinyUSB_Arduino) - USB HID stack (included with arduino-pico)
- [W55RP20-EVB-Pico Ethernet3 Lib](https://github.com/WIZnet-ioNIC/W55RP20-Ethernet3/) - Ethernet library
- [BearSSL](https://bearssl.org/) - TLS implementation (included with arduino-pico)

## License

MIT License - See [LICENSE](LICENSE) file.

## Contributing

Contributions welcome! Please open an issue or submit a pull request.

## Acknowledgments

- Built for humanoid robots who prefer not to touch keyboards... 😎
