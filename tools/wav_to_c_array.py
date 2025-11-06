#!/usr/bin/env python3
"""
Audio to C Array Converter
Converts WAV files to C arrays for embedding in ESP32 projects

Usage: python wav_to_c_array.py input.wav output.h array_name
"""

import sys
import wave
import struct

def wav_to_c_array(wav_file, header_file, array_name):
    try:
        # Open WAV file
        with wave.open(wav_file, 'rb') as wav:
            # Get audio parameters
            frames = wav.getnframes()
            sample_rate = wav.getframerate()
            channels = wav.getnchannels()
            sample_width = wav.getsampwidth()
            
            print(f"Audio info:")
            print(f"  Sample rate: {sample_rate} Hz")
            print(f"  Channels: {channels}")
            print(f"  Sample width: {sample_width} bytes")
            print(f"  Frames: {frames}")
            print(f"  Duration: {frames/sample_rate:.2f} seconds")
            
            # Read audio data
            audio_data = wav.readframes(frames)
            
        # Convert to 8-bit unsigned for ESP32 DAC
        samples = []
        if sample_width == 1:
            # 8-bit audio
            samples = list(audio_data)
        elif sample_width == 2:
            # 16-bit audio - convert to 8-bit
            for i in range(0, len(audio_data), 2):
                sample = struct.unpack('<h', audio_data[i:i+2])[0]
                # Convert from signed 16-bit (-32768 to 32767) to unsigned 8-bit (0 to 255)
                sample_8bit = int((sample + 32768) / 256)
                samples.append(sample_8bit)
        
        # If stereo, convert to mono by averaging channels
        if channels == 2:
            mono_samples = []
            for i in range(0, len(samples), 2):
                if i + 1 < len(samples):
                    avg = (samples[i] + samples[i + 1]) // 2
                    mono_samples.append(avg)
            samples = mono_samples
        
        # Write C header file
        with open(header_file, 'w') as f:
            f.write(f"// Auto-generated audio data from {wav_file}\n")
            f.write(f"// Sample rate: {sample_rate} Hz, Samples: {len(samples)}\n\n")
            f.write(f"#ifndef {array_name.upper()}_H\n")
            f.write(f"#define {array_name.upper()}_H\n\n")
            f.write(f"#include <AudioControl.h>\n\n")
            f.write(f"const unsigned int {array_name}_sample_rate = {sample_rate};\n")
            f.write(f"const unsigned int {array_name}_length = {len(samples)};\n\n")
            f.write(f"const unsigned char {array_name}_data[] = {{\n")
            
            # Write data in rows of 16 bytes
            for i in range(0, len(samples), 16):
                row = samples[i:i+16]
                f.write("  ")
                f.write(", ".join(f"0x{x:02x}" for x in row))
                if i + 16 < len(samples):
                    f.write(",")
                f.write("\n")
            
            f.write("};\n\n")
            f.write(f"// Bundled audio data for easy use\n")
            f.write(f"const AudioData {array_name} = {{\n")
            f.write(f"  {array_name}_data,\n")
            f.write(f"  {array_name}_length,\n")
            f.write(f"  {array_name}_sample_rate\n")
            f.write(f"}};\n\n")
            f.write(f"#endif // {array_name.upper()}_H\n")
        
        print(f"\nC array written to {header_file}")
        print(f"Simplified usage: play_audio({array_name});")
        print(f"Legacy access: {array_name}_data, {array_name}_length, {array_name}_sample_rate")
        
    except Exception as e:
        print(f"Error: {e}")
        return False
    
    return True

if __name__ == "__main__":
    if len(sys.argv) < 2 or len(sys.argv) > 4:
        print("Usage: python wav_to_c_array.py input.wav [output.h] [array_name]")
        print("Examples:")
        print("  python wav_to_c_array.py magic_sound.wav")
        print("    -> Creates magic_sound.h with array name 'magic_sound'")
        print("  python wav_to_c_array.py magic_sound.wav custom_output.h")
        print("    -> Creates custom_output.h with array name 'magic_sound'")
        print("  python wav_to_c_array.py magic_sound.wav custom_output.h my_array")
        print("    -> Creates custom_output.h with array name 'my_array'")
        sys.exit(1)
    
    wav_file = sys.argv[1]
    
    # Derive base name from input file (without extension)
    import os
    base_name = os.path.splitext(os.path.basename(wav_file))[0]
    
    # Use provided arguments or derive from base name
    header_file = sys.argv[2] if len(sys.argv) >= 3 else f"{base_name}.h"
    array_name = sys.argv[3] if len(sys.argv) >= 4 else base_name
    
    if wav_to_c_array(wav_file, header_file, array_name):
        print("Conversion successful!")
    else:
        print("Conversion failed!")
        sys.exit(1)