# OTA (Over-The-Air) Update Instructions

## Overview
Your MagiQuest box now supports wireless firmware updates via OTA (Over-The-Air). This means you can upload new firmware without physically connecting the ESP32 to your computer via USB cable.

## Prerequisites
- ESP32 must be powered on and connected to WiFi (OrbiMesh network)
- Computer must be on the same network as the ESP32
- Initial OTA-enabled firmware must be uploaded via USB (already completed)

## OTA Configuration
- **Hostname**: `MagiQuest-Box.local`
- **Password**: `magiquest2025`
- **Port**: `3232`

## Visual Feedback During OTA Update
The LED strip provides visual feedback during OTA updates:
- **White LEDs** (brightness 50): OTA update starting
- **Blue pulsing LEDs**: Update in progress (pulses based on download percentage)
- **Green flash** (3x): Update completed successfully
- **Red flash** (5x): Update failed - error occurred

## How to Upload Firmware via OTA

### Method 1: PlatformIO CLI (Recommended)
From the project directory, run:
```powershell
platformio run --target upload --upload-port MagiQuest-Box.local
```

Or with explicit password (if prompted):
```powershell
$env:PLATFORMIO_UPLOAD_FLAGS="--auth=magiquest2025"
platformio run --target upload --upload-port MagiQuest-Box.local
```

### Method 2: VS Code Task
1. Open Command Palette (Ctrl+Shift+P)
2. Type "Tasks: Run Task"
3. Select "PlatformIO: Upload (OTA)"
4. Watch the LED strip for visual feedback

### Method 3: Manual esptool OTA
If you need to manually specify all parameters:
```powershell
python -m espota --ip MagiQuest-Box.local --port 3232 --auth magiquest2025 --file .pio\build\esp32dev\firmware.bin
```

## Troubleshooting

### Device Not Found
If `MagiQuest-Box.local` cannot be found:
1. Check Serial Monitor for the IP address (it's printed on startup)
2. Use the IP address directly instead of hostname:
   ```powershell
   platformio run --target upload --upload-port 192.168.1.XXX
   ```

### Upload Fails
1. **Check WiFi connection**: Ensure ESP32 is connected to WiFi (check Serial Monitor)
2. **Verify same network**: Computer and ESP32 must be on OrbiMesh network
3. **Check mDNS**: Some routers don't support mDNS (.local hostnames) - use IP address instead
4. **Red LED flash**: If you see red flashing, check Serial Monitor for error details

### Password Issues
If password authentication fails, verify the password in `lib/OTAControl/OTAControl.h`:
```cpp
#define OTA_PASSWORD "magiquest2025"
```

### Still Not Working?
Fall back to USB upload:
```powershell
platformio run --target upload
```
This will overwrite with the new firmware, maintaining OTA capability.

## Security Notes
- Change the default password in `OTAControl.h` for production use
- OTA updates are only possible when ESP32 is on the same local network
- The password is transmitted but the update process is relatively secure for home use

## Advantages of OTA
- No need to physically access the ESP32
- Useful when device is mounted/enclosed
- Faster development cycle for testing
- Can update devices in hard-to-reach locations

## First Update After USB Upload
After the initial USB upload (which you just completed), all future updates can be done wirelessly via OTA!
