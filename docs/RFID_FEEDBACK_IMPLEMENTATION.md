# RFID Detection with Visual and Audio Feedback

## Implementation Guide

This guide shows how to implement the enhanced RFID detection with accelerating chase animation and success/fail feedback in your `main.cpp`.

## Code Example for main.cpp loop()

Replace your RFID detection code with this enhanced version:

```cpp
void loop() {
  
  // Handle OTA update requests (must be called frequently)
  loop_ota();
  
  // Process Home Assistant connection and commands
  loop_home_assistant();
  
  // Update stats for Home Assistant
  ha_stats.lid_is_open = false;  // No servo/lid
  ha_stats.time_until_ready = (millis() - last_activation < get_ha_cooldown()) ? 
    (get_ha_cooldown() - (millis() - last_activation)) / 1000 : 0;
  
  // Check if system is enabled via Home Assistant
  if (!is_system_enabled()) {
    delay(MAIN_LOOP_DELAY);
    return; // Skip band detection if disabled
  }
  
  // Apply Home Assistant brightness setting
  uint8_t ha_brightness = get_ha_brightness();
  if (ha_brightness != FastLED.getBrightness()) {
    FastLED.setBrightness(ha_brightness);
  }
  
  // Check for RFID input
  uint32_t band_id = loop_rfid();
  
  // Get current time
  unsigned long current_time = millis();
  
  // Use HA-controlled cooldown period
  unsigned long cooldown = get_ha_cooldown();
  
  // ====== ENHANCED RFID DETECTION WITH FEEDBACK ======
  if (band_id != 0 && current_time - last_activation >= cooldown) {
    
    // STEP 1: Show accelerating chase animation (builds excitement)
    DEBUG_PRINTLN("RFID detected - starting animation sequence");
    accelerating_chase(CRGB::White);  // White chase for detection
    
    // STEP 2: Search for matching band configuration
    bool band_found = false;
    for (int i = 0; i < NUM_BANDS; i++) {
      if (band_id == BAND_CONFIGS[i].band_id) {
        // ===== RECOGNIZED BAND - SUCCESS FEEDBACK =====
        DEBUG_PRINT("✓ Recognized RFID Band (ID: 0x");
        DEBUG_PRINT(band_id, HEX);
        DEBUG_PRINTLN(")");
        
        // Success animation: Fade in green, then fade out
        fade_in_out(CRGB::Green, 15);
        
        // Play success sound
        if (dfplayer_is_ready()) {
          play_sound_file(SOUND_RFID_SUCCESS);
        }
        
        delay(200);  // Brief pause between success feedback and character effect
        
        // Now show character-specific effect
        DEBUG_PRINT("Activating character - Sound variation: ");
        DEBUG_PRINTLN(BAND_CONFIGS[i].current_sound_index + 1);
        
        // Set character color
        set_color(BAND_CONFIGS[i].led_color);
        
        // Play character's sound variation
        uint8_t sound_file = BAND_CONFIGS[i].sound_files[BAND_CONFIGS[i].current_sound_index];
        if (dfplayer_is_ready()) {
          play_sound_file(sound_file);
        }
        
        // Rotate to next sound for next activation
        BAND_CONFIGS[i].current_sound_index = 
          (BAND_CONFIGS[i].current_sound_index + 1) % BAND_CONFIGS[i].num_sounds;
        
        // Publish band activation to Home Assistant
        publish_wand_activation(band_id);
        
        band_found = true;
        break;
      }
    }
    
    // ===== UNRECOGNIZED BAND - FAIL FEEDBACK =====
    if (!band_found) {
      DEBUG_PRINT("✗ Unknown RFID Band ID: 0x");
      DEBUG_PRINTLN(band_id, HEX);
      DEBUG_PRINTLN("To add this band:");
      DEBUG_PRINTLN("  1. Note the UID above");
      DEBUG_PRINTLN("  2. Add to lib/RFIDControl/RFIDControl.h");
      DEBUG_PRINTLN("  3. Add to BAND_CONFIGS[] in main.cpp");
      
      // Fail animation: Flash blue 3 times
      flash_color(CRGB::Blue, 3, 200);
      
      // Play fail/error sound
      if (dfplayer_is_ready()) {
        play_sound_file(SOUND_RFID_FAIL);
      }
    }
    
    last_activation = current_time; // Update last activation time
    
  } else if (band_id != 0) {
    DEBUG_PRINTLN("Band on cooldown - ignoring");
  }
  
  // Show cooldown visual feedback if in cooldown period
  if (current_time - last_activation < cooldown && last_activation > 0) {
    cooldown_pulse();
  }
  
  // wait a bit, and then back to receiving and decoding
  delay(MAIN_LOOP_DELAY);
}
```

## Animation Sequence Breakdown

### When RFID is Detected:

1. **Accelerating Chase** (White)
   - Starts slow (150ms per LED)
   - Progressively speeds up through 8 speed stages
   - Ends fast (10ms per LED)
   - Creates anticipation and excitement

2. **If Band is Recognized (SUCCESS):**
   - ✅ **Fade in Green** (smooth fade up)
   - **Hold green** for 500ms
   - **Fade out** (smooth fade down)
   - 🔊 **Play success sound** (chimes)
   - Brief 200ms pause
   - Show **character-specific color**
   - 🔊 **Play character sound**

3. **If Band is NOT Recognized (FAIL):**
   - ❌ **Flash Blue** 3 times (200ms on/off)
   - 🔊 **Play error sound** (box_closing)
   - Helpful debug message with instructions

## New LED Functions Available

### `accelerating_chase(CRGB color)`
- Chase that starts slow and speeds up
- Perfect for building excitement during detection

### `fade_in_out(CRGB color, int fade_speed_ms = 20)`
- Smooth fade in to full brightness
- Hold for 500ms
- Smooth fade out
- Perfect for success indication

### `flash_color(CRGB color, int num_flashes = 3, int flash_speed_ms = 200)`
- Flash color on/off multiple times
- Perfect for error/fail indication

## Sound Constants Added

In `AudioControlDFPlayer.h`:
```cpp
SOUND_RFID_SUCCESS = SOUND_CHIMES      // Success chime
SOUND_RFID_FAIL = SOUND_BOX_CLOSING    // Error sound
```

You can change these to use different sounds if you prefer.

## Customization Options

### Adjust Chase Speed
```cpp
// Modify speeds[] array in accelerating_chase() function
int speeds[] = {200, 150, 100, 50, 25};  // Slower start
```

### Change Success Color
```cpp
fade_in_out(CRGB::Purple, 15);  // Use purple instead of green
```

### More/Fewer Flashes for Fail
```cpp
flash_color(CRGB::Red, 5, 150);  // 5 red flashes, faster
```

### Different Feedback Sounds
```cpp
// In AudioControlDFPlayer.h, change:
SOUND_RFID_SUCCESS = SOUND_SPARKLE     // Use sparkle for success
SOUND_RFID_FAIL = SOUND_SWOOSH         // Use swoosh for fail
```

## Timing Summary

- **Accelerating chase**: ~2-3 seconds
- **Success feedback**: ~2 seconds (fade + sound)
- **Fail feedback**: ~1.5 seconds (3 flashes + sound)
- **Total detection-to-character**: ~4-5 seconds for success
- **Total detection-to-ready**: ~4 seconds for fail

## Testing

1. **Test with recognized band**:
   - Should see: White chase → Green fade → Character color
   - Should hear: Success chime → Character sound

2. **Test with unrecognized band**:
   - Should see: White chase → Blue flashes
   - Should hear: Error sound
   - Should see: Debug message with UID

3. **Test cooldown**:
   - Scan same band twice quickly
   - Second scan should be ignored with "on cooldown" message

## Benefits

✅ **Clear visual feedback** - User knows immediately if band was recognized
✅ **Exciting buildup** - Chase animation builds anticipation
✅ **Audio confirmation** - Sound reinforces visual feedback
✅ **Error handling** - Unknown bands get helpful feedback
✅ **Professional UX** - Smooth animations and clear states
