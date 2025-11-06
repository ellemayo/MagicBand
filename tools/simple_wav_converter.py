#!/usr/bin/env python3
"""
Simple, robust WAV to C array converter for ESP32 DAC
Focuses on correctness over fancy processing

Usage: python simple_wav_converter.py input.wav output.h array_name
"""

import sys
import wave
import struct

def simple_wav_to_c(wav_file, header_file, array_name, target_rate=8000):
    print(f"\n=== Simple WAV Converter ===")
    print(f"Input: {wav_file}")
    print(f"Output: {header_file}")
    print(f"Array name: {array_name}\n")
    
    # Open WAV file
    with wave.open(wav_file, 'rb') as wav:
        channels = wav.getnchannels()
        sample_width = wav.getsampwidth()
        framerate = wav.getframerate()
        n_frames = wav.getnframes()
        
        print(f"Input WAV properties:")
        print(f"  Channels: {channels}")
        print(f"  Sample width: {sample_width} bytes")
        print(f"  Sample rate: {framerate} Hz")
        print(f"  Total frames: {n_frames}")
        print(f"  Duration: {n_frames/framerate:.2f} seconds\n")
        
        # Read all audio data
        audio_bytes = wav.readframes(n_frames)
    
    # Convert to list of samples
    samples = []
    
    if sample_width == 1:
        # 8-bit unsigned (0-255)
        print("Processing 8-bit unsigned audio...")
        for i in range(0, len(audio_bytes), channels):
            # Get first channel (or average if stereo)
            if channels == 1:
                sample = audio_bytes[i]
            else:
                # Average all channels
                avg = sum(audio_bytes[i:i+channels]) / channels
                sample = int(avg)
            samples.append(sample)
            
    elif sample_width == 2:
        # 16-bit signed
        print("Processing 16-bit signed audio...")
        for i in range(0, len(audio_bytes), 2 * channels):
            # Get first channel sample
            frame = audio_bytes[i:i+2*channels]
            if channels == 1:
                sample_16 = struct.unpack('<h', frame[0:2])[0]
            else:
                # Average all channels
                channel_samples = []
                for ch in range(channels):
                    s = struct.unpack('<h', frame[ch*2:(ch+1)*2])[0]
                    channel_samples.append(s)
                sample_16 = sum(channel_samples) // channels
            
            # Convert 16-bit signed (-32768 to 32767) to 8-bit unsigned (0-255)
            sample = int((sample_16 + 32768) / 256)
            sample = max(0, min(255, sample))  # Clamp
            samples.append(sample)
    else:
        raise ValueError(f"Unsupported sample width: {sample_width}")
    
    print(f"Converted to {len(samples)} mono samples")
    
    # Simple resampling if needed (basic decimation/interpolation)
    if framerate != target_rate:
        print(f"Resampling from {framerate}Hz to {target_rate}Hz...")
        ratio = framerate / target_rate
        resampled = []
        for i in range(int(len(samples) / ratio)):
            src_idx = int(i * ratio)
            resampled.append(samples[src_idx])
        samples = resampled
        print(f"Resampled to {len(samples)} samples")
    
    # Analyze the data
    min_val = min(samples)
    max_val = max(samples)
    avg_val = sum(samples) / len(samples)
    
    print(f"\nSample statistics:")
    print(f"  Min: {min_val}")
    print(f"  Max: {max_val}")
    print(f"  Average: {avg_val:.1f}")
    print(f"  Range: {max_val - min_val}")
    print(f"  Center offset: {avg_val - 128:.1f} (should be near 0)")
    
    # Check for issues
    if max_val - min_val < 50:
        print("  ⚠️  WARNING: Very low dynamic range!")
    if abs(avg_val - 128) > 30:
        print("  ⚠️  WARNING: Significant DC offset!")
    
    # Write header file
    print(f"\nWriting to {header_file}...")
    with open(header_file, 'w') as f:
        f.write(f"// Generated from {wav_file}\n")
        f.write(f"// Simple conversion - {len(samples)} samples at {target_rate}Hz\n\n")
        f.write(f"#ifndef {array_name.upper()}_H\n")
        f.write(f"#define {array_name.upper()}_H\n\n")
        f.write(f"#include <AudioControl.h>\n\n")
        
        f.write(f"const unsigned int {array_name}_sample_rate = {target_rate};\n")
        f.write(f"const unsigned int {array_name}_length = {len(samples)};\n\n")
        
        f.write(f"const unsigned char {array_name}_data[] PROGMEM = {{\n")
        
        # Write data in rows of 16 bytes
        for i in range(0, len(samples), 16):
            row = samples[i:i+16]
            f.write("  ")
            f.write(", ".join(f"0x{x:02x}" for x in row))
            if i + 16 < len(samples):
                f.write(",")
            f.write("\n")
        
        f.write("};\n\n")
        
        f.write(f"const AudioData {array_name} = {{\n")
        f.write(f"  {array_name}_data,\n")
        f.write(f"  {array_name}_length,\n")
        f.write(f"  {array_name}_sample_rate\n")
        f.write(f"}};\n\n")
        f.write(f"#endif // {array_name.upper()}_H\n")
    
    print(f"✓ Conversion complete!")
    print(f"  Duration: {len(samples)/target_rate:.2f} seconds\n")
    
    # Show first 20 samples for debugging
    print("First 20 samples:", samples[:20])
    print("Last 20 samples:", samples[-20:])
    
    return True

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Usage: python simple_wav_converter.py input.wav output.h array_name [sample_rate]")
        sys.exit(1)
    
    wav_file = sys.argv[1]
    header_file = sys.argv[2]
    array_name = sys.argv[3]
    target_rate = int(sys.argv[4]) if len(sys.argv) > 4 else 8000
    
    try:
        simple_wav_to_c(wav_file, header_file, array_name, target_rate)
    except Exception as e:
        print(f"\n❌ Error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
