# Home Assistant / ESPHome Integration Configuration

## Quick Setup Guide

### 1. Configure WiFi and MQTT Credentials

Edit `lib/HomeAssistantControl/HomeAssistantControl.h` and update these lines:

```cpp
// WiFi Configuration
#define WIFI_SSID "YOUR_WIFI_SSID"           // Your WiFi network name
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"   // Your WiFi password

// MQTT Configuration (Home Assistant)
#define MQTT_SERVER "YOUR_HA_IP"             // Home Assistant IP address (e.g., "192.168.1.100")
#define MQTT_PORT 1883                       // Default MQTT port
#define MQTT_USER "YOUR_MQTT_USER"           // MQTT username (optional, use "" if none)
#define MQTT_PASSWORD "YOUR_MQTT_PASSWORD"   // MQTT password (optional, use "" if none)
```

### 2. Enable MQTT in Home Assistant

If you haven't already, install and configure the Mosquitto MQTT broker add-on in Home Assistant:

1. Go to **Settings** → **Add-ons** → **Add-on Store**
2. Search for "Mosquitto broker" and install it
3. Start the add-on and enable "Start on boot"
4. Create MQTT user credentials in **Settings** → **People** → **Users**

### 3. Upload Firmware

After configuring credentials:

```bash
pio run --target upload
```

### 4. Verify Connection

Open Serial Monitor at 115200 baud to see connection status:
- WiFi connection status and IP address
- MQTT connection status
- Home Assistant integration ready message

---

## Home Assistant Entities

Once connected, the following entities will automatically appear in Home Assistant:

### Controls (Writable)

| Entity | Type | Description | Range |
|--------|------|-------------|-------|
| `switch.magiquest_system` | Switch | Enable/disable wand detection | ON/OFF |
| `number.magiquest_brightness` | Number | LED strip brightness | 0-255 |
| `number.magiquest_cooldown` | Number | Cooldown time between activations | 1000-60000 ms |

### Sensors (Read-only)

| Entity | Type | Description |
|--------|------|-------------|
| `sensor.magiquest_last_wand` | Sensor | ID and name of last wand used |
| `sensor.magiquest_activations` | Sensor | Total wand activation count |
| `sensor.magiquest_uptime` | Sensor | System uptime in seconds |
| `sensor.magiquest_lid_state` | Binary Sensor | Current lid position (open/closed) |

---

## Example Automations

### Disable System at Bedtime

```yaml
automation:
  - alias: "MagiQuest Bedtime Disable"
    trigger:
      - platform: time
        at: "22:00:00"
    action:
      - service: switch.turn_off
        target:
          entity_id: switch.magiquest_system
```

### Re-enable in Morning

```yaml
automation:
  - alias: "MagiQuest Morning Enable"
    trigger:
      - platform: time
        at: "08:00:00"
    action:
      - service: switch.turn_on
        target:
          entity_id: switch.magiquest_system
```

### Notification on Wand Use

```yaml
automation:
  - alias: "MagiQuest Wand Activity Notification"
    trigger:
      - platform: state
        entity_id: sensor.magiquest_last_wand
    action:
      - service: notify.mobile_app
        data:
          message: "{{ trigger.to_state.attributes.name }} wand was used!"
```

### Dim LEDs at Night

```yaml
automation:
  - alias: "MagiQuest Night Mode"
    trigger:
      - platform: sun
        event: sunset
    action:
      - service: number.set_value
        target:
          entity_id: number.magiquest_brightness
        data:
          value: 5
```

### Restore Brightness in Morning

```yaml
automation:
  - alias: "MagiQuest Day Mode"
    trigger:
      - platform: sun
        event: sunrise
    action:
      - service: number.set_value
        target:
          entity_id: number.magiquest_brightness
        data:
          value: 20
```

---

## Lovelace Dashboard Card Example

```yaml
type: entities
title: MagiQuest Box
entities:
  - entity: switch.magiquest_system
    name: System Enabled
  - entity: number.magiquest_brightness
    name: LED Brightness
  - entity: number.magiquest_cooldown
    name: Cooldown (ms)
  - entity: sensor.magiquest_last_wand
    name: Last Wand
  - entity: sensor.magiquest_activations
    name: Total Activations
```

---

## Troubleshooting

### WiFi Not Connecting
- Verify SSID and password are correct
- Check WiFi signal strength at ESP32 location
- Ensure 2.4GHz WiFi is enabled (ESP32 doesn't support 5GHz)

### MQTT Not Connecting
- Verify Home Assistant IP address is correct
- Check MQTT broker is running in HA
- Verify MQTT credentials match HA user
- Check firewall isn't blocking port 1883

### Entities Not Appearing in Home Assistant
- Check Serial Monitor for "Discovery configs published" message
- Wait 1-2 minutes for HA to discover entities
- Restart Home Assistant if needed
- Check **Settings** → **Devices & Services** → **MQTT** in HA

### System Performance
- Home Assistant integration runs asynchronously
- WiFi/MQTT operations don't block wand detection
- If experiencing issues, you can disable HA integration by commenting out `setup_home_assistant()` and `loop_home_assistant()` calls in `main.cpp`

---

## MQTT Topics Reference

For advanced users who want to integrate with other systems:

| Topic | Direction | Description |
|-------|-----------|-------------|
| `homeassistant/magiquest/status` | Publish | Online/offline status |
| `homeassistant/magiquest/command` | Subscribe | System ON/OFF commands |
| `homeassistant/magiquest/state` | Publish | Current system state (JSON) |
| `homeassistant/magiquest/wand` | Publish | Wand activation events (JSON) |
| `homeassistant/magiquest/brightness/set` | Subscribe | Set LED brightness |
| `homeassistant/magiquest/cooldown/set` | Subscribe | Set cooldown time |
| `homeassistant/magiquest/stats` | Publish | System statistics (JSON) |

---

## Architecture Notes

The Home Assistant integration follows the same modular architecture as other system components:

- **Library Module**: `lib/HomeAssistantControl/`
- **Setup Function**: `setup_home_assistant()` - Called once during boot
- **Loop Function**: `loop_home_assistant()` - Called every iteration
- **Non-blocking**: WiFi and MQTT operations don't interfere with IR detection
- **Graceful Degradation**: System works normally even if WiFi/MQTT fails

The integration exposes control variables that other modules check:
- `is_system_enabled()` - Check if system is enabled
- `get_ha_brightness()` - Get current brightness setting
- `get_ha_cooldown()` - Get current cooldown period
- `is_auto_close_enabled()` - Check if auto-close is enabled
