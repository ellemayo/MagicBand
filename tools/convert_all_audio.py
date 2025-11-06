#!/usr/bin/env python3
"""
Batch convert all WAV files in assets/audio to C headers
Uses the simple, robust converter
"""

import os
import sys

# Add tools directory to path
sys.path.insert(0, 'tools')
from simple_wav_converter import simple_wav_to_c

# Audio files to convert
audio_files = [
    ('assets/audio/sparkle-min.wav', 'lib/AudioControl/sounds/sparkle.h', 'sparkle'),
    ('assets/audio/box_opening.wav', 'lib/AudioControl/sounds/box_opening.h', 'box_opening'),
    ('assets/audio/box_closing.wav', 'lib/AudioControl/sounds/box_closing.h', 'box_closing'),
    ('assets/audio/chimes.wav', 'lib/AudioControl/sounds/chimes.h', 'chimes'),
    ('assets/audio/swoosh.wav', 'lib/AudioControl/sounds/swoosh.h', 'swoosh'),
    ('assets/audio/wand.wav', 'lib/AudioControl/sounds/wand.h', 'wand'),
]

TARGET_SAMPLE_RATE = 8000

print("="*60)
print("BATCH AUDIO CONVERTER FOR ESP32")
print("="*60)

success_count = 0
fail_count = 0

for wav_file, header_file, array_name in audio_files:
    if not os.path.exists(wav_file):
        print(f"\n⚠️  Skipping {wav_file} (file not found)")
        continue
    
    try:
        print(f"\n{'='*60}")
        simple_wav_to_c(wav_file, header_file, array_name, TARGET_SAMPLE_RATE)
        success_count += 1
    except Exception as e:
        print(f"\n❌ Failed to convert {wav_file}: {e}")
        fail_count += 1

print(f"\n{'='*60}")
print(f"BATCH CONVERSION COMPLETE")
print(f"{'='*60}")
print(f"✓ Success: {success_count}")
print(f"✗ Failed: {fail_count}")
print(f"\nAll converted files use 8kHz sample rate and are ready for ESP32 DAC playback.")
