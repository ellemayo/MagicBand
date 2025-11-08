#ifndef HOME_ASSISTANT_CONTROL_H
#define HOME_ASSISTANT_CONTROL_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// WiFi Configuration
#define WIFI_SSID "OrbiMesh"
#define WIFI_PASSWORD "05082013"

// MQTT Configuration (Home Assistant)
#define MQTT_SERVER "homeassistant.local"  // Try IP if .local doesn't work
#define MQTT_PORT 1883
#define MQTT_USER "smarts"
#define MQTT_PASSWORD "uyw5khj.jqw3ZVC2tdp"
#define MQTT_CLIENT_ID "magicband"

// MQTT Topics
#define MQTT_BASE_TOPIC "homeassistant/magicband"
#define MQTT_STATUS_TOPIC MQTT_BASE_TOPIC "/status"
#define MQTT_COMMAND_TOPIC MQTT_BASE_TOPIC "/command"
#define MQTT_STATE_TOPIC MQTT_BASE_TOPIC "/state"
#define MQTT_WAND_TOPIC MQTT_BASE_TOPIC "/wand"
#define MQTT_BRIGHTNESS_TOPIC MQTT_BASE_TOPIC "/brightness"
#define MQTT_COOLDOWN_TOPIC MQTT_BASE_TOPIC "/cooldown"
#define MQTT_STATS_TOPIC MQTT_BASE_TOPIC "/stats"

// Home Assistant Discovery Topics
#define HA_DISCOVERY_PREFIX "homeassistant"

// Control flags that can be modified by Home Assistant
struct HAControlState {
  bool system_enabled;          // Enable/disable wand detection
  uint8_t led_brightness;       // LED brightness 0-255
  unsigned long cooldown_time;  // Cooldown period in ms
  bool auto_close_enabled;      // Enable/disable auto-close
};

// Statistics to report to Home Assistant
struct HAStats {
  uint32_t last_wand_id;        // Last wand that was activated
  unsigned long activation_count; // Total activation count
  unsigned long uptime;         // System uptime in seconds
  bool lid_is_open;             // Current lid state
  unsigned long time_until_ready; // Time until cooldown ends
};

// Global control state
extern HAControlState ha_control;
extern HAStats ha_stats;

// Function declarations
void setup_home_assistant();
void loop_home_assistant();
void publish_discovery_configs();
void publish_state();
void publish_wand_activation(uint32_t wand_id);
void publish_stats();
void reconnect_mqtt();
void mqtt_callback(char* topic, byte* payload, unsigned int length);

// Helper functions to check states
bool is_system_enabled();
uint8_t get_ha_brightness();
unsigned long get_ha_cooldown();
bool is_auto_close_enabled();

#endif // HOME_ASSISTANT_CONTROL_H
