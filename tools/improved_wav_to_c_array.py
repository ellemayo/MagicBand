#!/usr/bin/env python3
"""
Improved Audio to C Array Converter for ESP32
Optimized for ESP32 DAC limitations and better quality

Usage: python improved_wav_to_c_array.py input.wav output.h array_name [--downsample]
"""

import sys
import wave
import struct
import numpy as np
from scipy import signal
import argparse

def resample_audio(data, original_rate, target_rate):
    """Resample audio data using high-quality resampling"""
    if original_rate == target_rate:
        return data
    
    # Calculate resampling ratio
    ratio = target_rate / original_rate
    
    # Use scipy's high-quality resampling
    resampled = signal.resample(data, int(len(data) * ratio))
    return resampled.astype(np.int16)

def apply_audio_filters(data, sample_rate):
    """Apply audio processing to improve quality"""
    # Convert to float for processing
    audio_float = data.astype(np.float32)
    
    # Normalize to prevent clipping
    max_val = np.max(np.abs(audio_float))
    if max_val > 0:
        audio_float = audio_float / max_val * 0.95  # Leave some headroom
    
    # Apply a slight low-pass filter to reduce aliasing
    # Cutoff at 3.5kHz for 8kHz sample rate (below Nyquist)
    nyquist = sample_rate / 2
    cutoff = min(3500, nyquist * 0.8)  # Conservative cutoff
    
    if cutoff < nyquist:
        sos = signal.butter(4, cutoff / nyquist, btype='low', output='sos')
        audio_float = signal.sosfilt(sos, audio_float)
    
    # Apply gentle compression to even out levels
    threshold = 0.7
    ratio = 3.0
    above_threshold = np.abs(audio_float) > threshold
    sign = np.sign(audio_float)
    magnitude = np.abs(audio_float)
    
    compressed_magnitude = np.where(
        above_threshold,
        threshold + (magnitude - threshold) / ratio,
        magnitude
    )
    audio_float = sign * compressed_magnitude
    
    # Convert back to int16
    audio_float = np.clip(audio_float * 32767, -32768, 32767)
    return audio_float.astype(np.int16)

def wav_to_c_array(wav_file, header_file, array_name, target_sample_rate=8000):
    try:
        print(f"Processing {wav_file}...")
        
        # Open WAV file
        with wave.open(wav_file, 'rb') as wav:
            # Get audio parameters
            frames = wav.getnframes()
            original_rate = wav.getframerate()
            channels = wav.getnchannels()
            sample_width = wav.getsampwidth()
            
            print(f"Original audio info:")
            print(f"  Sample rate: {original_rate} Hz")
            print(f"  Channels: {channels}")
            print(f"  Sample width: {sample_width} bytes")
            print(f"  Duration: {frames/original_rate:.2f} seconds")
            
            # Read audio data
            audio_data = wav.readframes(frames)
            
        # Convert to numpy array
        if sample_width == 1:
            # 8-bit audio - convert to signed
            samples = np.frombuffer(audio_data, dtype=np.uint8).astype(np.int16)
            samples = (samples - 128) * 256  # Convert to 16-bit signed
        elif sample_width == 2:
            # 16-bit audio
            samples = np.frombuffer(audio_data, dtype=np.int16)
        else:
            raise ValueError(f"Unsupported sample width: {sample_width}")
        
        # Handle stereo - convert to mono by averaging
        if channels == 2:
            samples = samples.reshape(-1, 2)
            samples = np.mean(samples, axis=1).astype(np.int16)
        
        # Resample to target rate if different
        if original_rate != target_sample_rate:
            print(f"Resampling from {original_rate}Hz to {target_sample_rate}Hz...")
            samples = resample_audio(samples, original_rate, target_sample_rate)
        
        # Apply audio processing
        print("Applying audio filters...")
        samples = apply_audio_filters(samples, target_sample_rate)
        
        # Convert to 8-bit unsigned for ESP32 DAC
        # Use better quantization with dithering
        samples_float = samples.astype(np.float32) / 32768.0  # Normalize to -1 to 1
        
        # Add subtle dithering to reduce quantization noise
        dither = np.random.normal(0, 1/256, len(samples_float))
        samples_float += dither
        
        # Convert to 8-bit unsigned (0-255) with better scaling
        samples_8bit = np.clip((samples_float + 1.0) * 127.5, 0, 255).astype(np.uint8)
        
        print(f"Final audio: {len(samples_8bit)} samples at {target_sample_rate}Hz")
        print(f"Duration: {len(samples_8bit)/target_sample_rate:.2f} seconds")
        
        # Write C header file
        with open(header_file, 'w') as f:
            f.write(f"// Auto-generated audio data from {wav_file}\n")
            f.write(f"// Processed for ESP32 DAC optimization\n")
            f.write(f"// Sample rate: {target_sample_rate} Hz, Samples: {len(samples_8bit)}\n\n")
            f.write(f"#ifndef {array_name.upper()}_H\n")
            f.write(f"#define {array_name.upper()}_H\n\n")
            f.write(f"#include <AudioControl.h>\n\n")
            f.write(f"const unsigned int {array_name}_sample_rate = {target_sample_rate};\n")
            f.write(f"const unsigned int {array_name}_length = {len(samples_8bit)};\n\n")
            f.write(f"const unsigned char {array_name}_data[] PROGMEM = {{\n")
            
            # Write data in rows of 16 bytes
            for i in range(0, len(samples_8bit), 16):
                row = samples_8bit[i:i+16]
                f.write("  ")
                f.write(", ".join(f"0x{x:02x}" for x in row))
                if i + 16 < len(samples_8bit):
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
        
        print(f"\nImproved C array written to {header_file}")
        print(f"Usage: play_audio({array_name});")
        
        return True
        
    except Exception as e:
        print(f"Error: {e}")
        import traceback
        traceback.print_exc()
        return False

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Convert WAV to optimized C array for ESP32')
    parser.add_argument('input_wav', help='Input WAV file')
    parser.add_argument('output_header', nargs='?', help='Output header file')
    parser.add_argument('array_name', nargs='?', help='Array name in C code')
    parser.add_argument('--sample-rate', type=int, default=8000, 
                       help='Target sample rate (default: 8000Hz)')
    
    args = parser.parse_args()
    
    # Check if required packages are available
    try:
        import numpy as np
        from scipy import signal
    except ImportError:
        print("Error: This script requires numpy and scipy.")
        print("Install with: pip install numpy scipy")
        sys.exit(1)
    
    wav_file = args.input_wav
    
    # Derive base name from input file (without extension)
    import os
    base_name = os.path.splitext(os.path.basename(wav_file))[0]
    
    # Use provided arguments or derive from base name
    header_file = args.output_header if args.output_header else f"{base_name}_optimized.h"
    array_name = args.array_name if args.array_name else f"{base_name}_opt"
    
    print(f"Converting {wav_file} to {header_file} with array name '{array_name}'")
    print(f"Target sample rate: {args.sample_rate}Hz")
    
    if wav_to_c_array(wav_file, header_file, array_name, args.sample_rate):
        print("Conversion successful!")
    else:
        print("Conversion failed!")
        sys.exit(1)