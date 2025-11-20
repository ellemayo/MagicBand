#include "HomeAssistantControl.h"
#include <DebugConfig.h>
#include <ArduinoJson.h>
#include <BandConfig.h>

// WiFi and MQTT clients
WiFiClient espClient;
PubSubClient mqtt_client(espClient);

// Global control state - initialize with defaults
HAControlState ha_control = {
  .system_enabled = true,      // System enabled by default
  .led_brightness = 80,        // Match LED_DEFAULT_BRIGHTNESS
  .cooldown_time = 5000,       // Match COOLDOWN_PERIOD
  .auto_close_enabled = true   // Auto-close enabled by default
};

HAStats ha_stats = {
  .last_wand_id = 0,
  .activation_count = 0,
  .uptime = 0,
  .lid_is_open = false,
  .time_until_ready = 0
};

unsigned long last_reconnect_attempt = 0;
unsigned long last_stats_publish = 0;
const unsigned long STATS_PUBLISH_INTERVAL = 30000; // Publish stats every 30 seconds

void setup_home_assistant() {
  DEBUG_PRINTLN("Setting up Home Assistant integration...");
  
  // Connect to WiFi - non-blocking with shorter timeout
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  DEBUG_PRINT("Connecting to WiFi");
  int wifi_attempts = 0;
  // Reduced from 20 to 10 attempts (5 seconds max instead of 10)
  while (WiFi.status() != WL_CONNECTED && wifi_attempts < 10) {
    delay(500);
    DEBUG_PRINT(".");
    wifi_attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    DEBUG_PRINTLN();
    DEBUG_PRINT("WiFi connected! IP: ");
    DEBUG_PRINTLN(WiFi.localIP());
    
    // Setup MQTT
    mqtt_client.setServer(MQTT_SERVER, MQTT_PORT);
    mqtt_client.setCallback(mqtt_callback);
    mqtt_client.setBufferSize(512); // Increase buffer for JSON messages
    
    // Try initial connection - but don't block if it fails
    // Will retry in loop_home_assistant()
    reconnect_mqtt();
  } else {
    DEBUG_PRINTLN();
    DEBUG_PRINTLN("WiFi connection timeout - will retry in background");
    DEBUG_PRINTLN("System will continue without Home Assistant for now");
    
    // Setup MQTT anyway so reconnection can work in loop
    mqtt_client.setServer(MQTT_SERVER, MQTT_PORT);
    mqtt_client.setCallback(mqtt_callback);
    mqtt_client.setBufferSize(512);
  }
}

void loop_home_assistant() {
  // Try to maintain WiFi connection - non-blocking
  if (WiFi.status() != WL_CONNECTED) {
    // Try to reconnect WiFi periodically but don't block
    static unsigned long last_wifi_check = 0;
    if (millis() - last_wifi_check > 30000) { // Check every 30 seconds
      last_wifi_check = millis();
      DEBUG_PRINTLN("WiFi disconnected - attempting reconnect...");
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }
    return; // Skip MQTT if WiFi not connected
  }
  
  // Maintain MQTT connection
  if (!mqtt_client.connected()) {
    unsigned long now = millis();
    if (now - last_reconnect_attempt > 5000) {
      last_reconnect_attempt = now;
      reconnect_mqtt();
    }
  } else {
    mqtt_client.loop();
    
    // Publish stats periodically
    unsigned long now = millis();
    if (now - last_stats_publish > STATS_PUBLISH_INTERVAL) {
      last_stats_publish = now;
      ha_stats.uptime = millis() / 1000;
      publish_stats();
    }
  }
}

void reconnect_mqtt() {
  DEBUG_PRINTLN("Attempting MQTT connection...");
  
  // mqtt_client.connect() can block, but it has internal timeout (~15 seconds)
  // This is acceptable since it only happens during reconnection attempts
  if (mqtt_client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD, MQTT_STATUS_TOPIC, 0, true, "offline")) {
    DEBUG_PRINTLN("MQTT connected!");
    
    // Publish online status
    mqtt_client.publish(MQTT_STATUS_TOPIC, "online", true);
    
    // Subscribe to command topics
    mqtt_client.subscribe(MQTT_COMMAND_TOPIC);
    mqtt_client.subscribe(MQTT_BRIGHTNESS_TOPIC "/set");
    mqtt_client.subscribe(MQTT_COOLDOWN_TOPIC "/set");
    
    // Publish Home Assistant discovery configs
    publish_discovery_configs();
    
    // Publish initial state
    publish_state();
    publish_stats();
    
    DEBUG_PRINTLN("Home Assistant integration ready");
  } else {
    DEBUG_PRINT("MQTT connection failed, rc=");
    DEBUG_PRINTLN(mqtt_client.state());
    DEBUG_PRINTLN("Will retry in 5 seconds");
  }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  // Convert payload to string
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  
  DEBUG_PRINT("MQTT message on ");
  DEBUG_PRINT(topic);
  DEBUG_PRINT(": ");
  DEBUG_PRINTLN(message);
  
  // Handle command topic
  if (strcmp(topic, MQTT_COMMAND_TOPIC) == 0) {
    if (strcmp(message, "ON") == 0 || strcmp(message, "on") == 0) {
      ha_control.system_enabled = true;
      DEBUG_PRINTLN("System enabled via Home Assistant");
    } else if (strcmp(message, "OFF") == 0 || strcmp(message, "off") == 0) {
      ha_control.system_enabled = false;
      DEBUG_PRINTLN("System disabled via Home Assistant");
    }
    publish_state();
  }
  // Handle brightness topic
  else if (strstr(topic, "brightness/set") != NULL) {
    int brightness = atoi(message);
    if (brightness >= 0 && brightness <= 255) {
      ha_control.led_brightness = brightness;
      DEBUG_PRINT("LED brightness set to: ");
      DEBUG_PRINTLN(brightness);
      publish_state();
    }
  }
  // Handle cooldown topic
  else if (strstr(topic, "cooldown/set") != NULL) {
    unsigned long cooldown = atol(message);
    if (cooldown >= 1000 && cooldown <= 60000) { // 1-60 seconds
      ha_control.cooldown_time = cooldown;
      DEBUG_PRINT("Cooldown time set to: ");
      DEBUG_PRINT(cooldown);
      DEBUG_PRINTLN("ms");
      publish_state();
    }
  }
}

void publish_discovery_configs() {
  DEBUG_PRINTLN("Publishing Home Assistant discovery configs...");
  
  StaticJsonDocument<512> doc;
  char buffer[512];
  char topic[128];
  
  // Switch entity for system enable/disable
  doc.clear();
  doc["name"] = "MagicBand System";
  doc["unique_id"] = "magicband_system";
  doc["state_topic"] = MQTT_STATE_TOPIC;
  doc["command_topic"] = MQTT_COMMAND_TOPIC;
  doc["payload_on"] = "ON";
  doc["payload_off"] = "OFF";
  doc["value_template"] = "{{ value_json.enabled }}";
  doc["device"]["identifiers"][0] = "magicband";
  doc["device"]["name"] = "MagicBand";
  doc["device"]["manufacturer"] = "Custom";
  doc["device"]["model"] = "ESP32";
  
  serializeJson(doc, buffer);
  snprintf(topic, sizeof(topic), "%s/switch/%s/config", HA_DISCOVERY_PREFIX, MQTT_CLIENT_ID);
  mqtt_client.publish(topic, buffer, true);
  
  // Number entity for LED brightness
  doc.clear();
  doc["name"] = "LED Brightness";
  doc["unique_id"] = "magicband_brightness";
  doc["state_topic"] = MQTT_STATE_TOPIC;
  doc["command_topic"] = MQTT_BRIGHTNESS_TOPIC "/set";
  doc["value_template"] = "{{ value_json.brightness }}";
  doc["min"] = 0;
  doc["max"] = 255;
  doc["device"]["identifiers"][0] = "magicband";
  
  serializeJson(doc, buffer);
  snprintf(topic, sizeof(topic), "%s/number/%s_brightness/config", HA_DISCOVERY_PREFIX, MQTT_CLIENT_ID);
  mqtt_client.publish(topic, buffer, true);
  
  // Number entity for cooldown time
  doc.clear();
  doc["name"] = "Cooldown Time";
  doc["unique_id"] = "magicband_cooldown";
  doc["state_topic"] = MQTT_STATE_TOPIC;
  doc["command_topic"] = MQTT_COOLDOWN_TOPIC "/set";
  doc["value_template"] = "{{ value_json.cooldown }}";
  doc["unit_of_measurement"] = "ms";
  doc["min"] = 1000;
  doc["max"] = 60000;
  doc["device"]["identifiers"][0] = "magicband";
  
  serializeJson(doc, buffer);
  snprintf(topic, sizeof(topic), "%s/number/%s_cooldown/config", HA_DISCOVERY_PREFIX, MQTT_CLIENT_ID);
  mqtt_client.publish(topic, buffer, true);
  
  // Sensor for last wand
  doc.clear();
  doc["name"] = "Last Wand";
  doc["unique_id"] = "magicband_last_wand";
  doc["state_topic"] = MQTT_WAND_TOPIC;
  doc["value_template"] = "{{ value_json.wand_id }}";
  doc["device"]["identifiers"][0] = "magicband";
  
  serializeJson(doc, buffer);
  snprintf(topic, sizeof(topic), "%s/sensor/%s_last_wand/config", HA_DISCOVERY_PREFIX, MQTT_CLIENT_ID);
  mqtt_client.publish(topic, buffer, true);
  
  // Sensor for activation count
  doc.clear();
  doc["name"] = "Activation Count";
  doc["unique_id"] = "magicband_activations";
  doc["state_topic"] = MQTT_STATS_TOPIC;
  doc["value_template"] = "{{ value_json.activations }}";
  doc["device"]["identifiers"][0] = "magicband";
  
  serializeJson(doc, buffer);
  snprintf(topic, sizeof(topic), "%s/sensor/%s_activations/config", HA_DISCOVERY_PREFIX, MQTT_CLIENT_ID);
  mqtt_client.publish(topic, buffer, true);
  
  DEBUG_PRINTLN("Discovery configs published");
}

void publish_state() {
  if (!mqtt_client.connected()) return;
  
  StaticJsonDocument<256> doc;
  char buffer[256];
  
  doc["enabled"] = ha_control.system_enabled ? "ON" : "OFF";
  doc["brightness"] = ha_control.led_brightness;
  doc["cooldown"] = ha_control.cooldown_time;
  doc["auto_close"] = ha_control.auto_close_enabled;
  
  serializeJson(doc, buffer);
  mqtt_client.publish(MQTT_STATE_TOPIC, buffer, true);
}

void publish_wand_activation(uint64_t wand_id) {
  if (!mqtt_client.connected()) return;
  
  ha_stats.last_wand_id = wand_id;  // Store full 64-bit value
  ha_stats.activation_count++;
  
  StaticJsonDocument<128> doc;
  char buffer[128];
  
  // Send full 64-bit ID as string to avoid JSON integer overflow
  char wand_id_str[20];
  sprintf(wand_id_str, "%llu", wand_id);
  doc["wand_id"] = wand_id_str;
  doc["timestamp"] = millis();
  
  // Look up band name from configuration
  BandConfig* band = find_band_config(wand_id);
  if (band != nullptr) {
    doc["name"] = band->name;
  } else {
    doc["name"] = "Unknown";
  }
  
  serializeJson(doc, buffer);
  mqtt_client.publish(MQTT_WAND_TOPIC, buffer);
  
  DEBUG_PRINT("Published wand activation: ");
  DEBUG_PRINTLN(wand_id_str);
}

void publish_stats() {
  if (!mqtt_client.connected()) return;
  
  StaticJsonDocument<256> doc;
  char buffer[256];
  
  doc["activations"] = ha_stats.activation_count;
  doc["uptime"] = ha_stats.uptime;
  doc["lid_open"] = ha_stats.lid_is_open;
  doc["time_until_ready"] = ha_stats.time_until_ready;
  
  // Send last_wand as string to avoid JSON integer overflow with 64-bit values
  char wand_id_str[20];
  sprintf(wand_id_str, "%llu", ha_stats.last_wand_id);
  doc["last_wand"] = wand_id_str;
  
  serializeJson(doc, buffer);
  mqtt_client.publish(MQTT_STATS_TOPIC, buffer);
}

// Helper functions
bool is_system_enabled() {
  return ha_control.system_enabled;
}

uint8_t get_ha_brightness() {
  return ha_control.led_brightness;
}

unsigned long get_ha_cooldown() {
  return ha_control.cooldown_time;
}

bool is_auto_close_enabled() {
  return ha_control.auto_close_enabled;
}
