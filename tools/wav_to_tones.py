#!/usr/bin/env python3
"""
Convert WAV files to tone sequences for ESP32
Analyzes audio and extracts dominant frequencies to recreate as tones

Usage: python wav_to_tones.py input.wav output.h function_name
"""

import sys
import wave
import numpy as np
from scipy import signal, fft
import argparse

def analyze_audio_segment(audio_data, sample_rate, segment_duration=0.05):
    """
    Analyze audio in small segments to extract frequency and amplitude over time
    Returns list of (frequency, amplitude, duration) tuples
    """
    segment_samples = int(segment_duration * sample_rate)
    num_segments = len(audio_data) // segment_samples
    
    tone_sequence = []
    
    for i in range(num_segments):
        start = i * segment_samples
        end = start + segment_samples
        segment = audio_data[start:end]
        
        if len(segment) < 10:  # Skip tiny segments
            continue
        
        # Calculate RMS amplitude
        rms = np.sqrt(np.mean(segment**2))
        
        # Skip very quiet segments
        if rms < 0.02:
            continue
        
        # Perform FFT to find dominant frequency
        fft_data = np.fft.rfft(segment)
        fft_freq = np.fft.rfftfreq(len(segment), 1/sample_rate)
        
        # Find peak frequency (ignoring DC component)
        magnitude = np.abs(fft_data[1:])
        peak_idx = np.argmax(magnitude) + 1
        dominant_freq = fft_freq[peak_idx]
        
        # Filter out unreasonable frequencies
        if dominant_freq < 50 or dominant_freq > 8000:
            continue
        
        # Round to nearest musical note if close
        dominant_freq = round_to_musical_note(dominant_freq)
        
        # Combine consecutive segments with same frequency
        if tone_sequence and abs(tone_sequence[-1][0] - dominant_freq) < 20:
            # Extend previous tone
            tone_sequence[-1] = (tone_sequence[-1][0], 
                                tone_sequence[-1][1], 
                                tone_sequence[-1][2] + segment_duration)
        else:
            # Add new tone
            tone_sequence.append((int(dominant_freq), rms, segment_duration))
    
    return tone_sequence

def round_to_musical_note(frequency):
    """Round frequency to nearest musical note for cleaner sound"""
    # A4 = 440Hz reference
    if abs(frequency - 440) < 20:
        return 440
    
    # Common musical notes in Hz
    notes = [
        262, 277, 294, 311, 330, 349, 370, 392, 415, 440,  # C4-A4
        466, 494, 523, 554, 587, 622, 659, 698, 740, 784,  # B4-G5
        831, 880, 932, 988, 1047, 1109, 1175, 1245, 1319, 1397  # A5-F6
    ]
    
    # Find closest note
    closest = min(notes, key=lambda x: abs(x - frequency))
    if abs(closest - frequency) < 30:
        return closest
    
    return frequency

def simplify_tone_sequence(tones, max_tones=15):
    """Simplify tone sequence by merging similar tones and limiting count"""
    if len(tones) <= max_tones:
        return tones
    
    # Merge very short tones with neighbors
    simplified = []
    i = 0
    while i < len(tones):
        freq, amp, dur = tones[i]
        
        # If tone is very short (< 30ms), try to merge
        if dur < 0.03 and simplified:
            # Extend previous tone
            simplified[-1] = (simplified[-1][0], 
                            simplified[-1][1], 
                            simplified[-1][2] + dur)
        else:
            simplified.append((freq, amp, dur))
        
        i += 1
    
    return simplified

def generate_tone_function(tones, function_name, description):
    """Generate C++ code for tone sequence"""
    code = f"""// Generated tone sequence: {description}
void play_{function_name}_tones() {{
  // Sequence of {len(tones)} tones
"""
    
    for freq, amp, duration in tones:
        duration_ms = int(duration * 1000)
        code += f"  play_tone({freq}, {duration_ms});  // {freq}Hz for {duration_ms}ms\n"
        
        # Add small pause between some tones for clarity
        if duration > 0.1:
            code += f"  delay(10);  // Brief pause\n"
    
    code += "}\n"
    return code

def wav_to_tones(wav_file, header_file, function_name):
    """Main conversion function"""
    try:
        print(f"\n{'='*60}")
        print(f"Analyzing {wav_file}...")
        print(f"{'='*60}\n")
        
        # Open WAV file
        with wave.open(wav_file, 'rb') as wav:
            frames = wav.getnframes()
            sample_rate = wav.getframerate()
            channels = wav.getnchannels()
            sample_width = wav.getsampwidth()
            duration = frames / sample_rate
            
            print(f"Audio Properties:")
            print(f"  Sample rate: {sample_rate} Hz")
            print(f"  Channels: {channels}")
            print(f"  Sample width: {sample_width} bytes")
            print(f"  Duration: {duration:.2f} seconds")
            print()
            
            # Read audio data
            audio_data = wav.readframes(frames)
        
        # Convert to numpy array
        if sample_width == 1:
            samples = np.frombuffer(audio_data, dtype=np.uint8).astype(np.float32)
            samples = (samples - 128) / 128.0  # Normalize to -1 to 1
        elif sample_width == 2:
            samples = np.frombuffer(audio_data, dtype=np.int16).astype(np.float32)
            samples = samples / 32768.0  # Normalize to -1 to 1
        else:
            raise ValueError(f"Unsupported sample width: {sample_width}")
        
        # Handle stereo - convert to mono
        if channels == 2:
            samples = samples.reshape(-1, 2)
            samples = np.mean(samples, axis=1)
        
        print("Analyzing frequency content...")
        
        # Analyze audio to extract tone sequence
        tones = analyze_audio_segment(samples, sample_rate, segment_duration=0.05)
        
        print(f"Found {len(tones)} distinct tone segments")
        
        # Simplify if too complex
        if len(tones) > 20:
            print(f"Simplifying tone sequence...")
            tones = simplify_tone_sequence(tones, max_tones=15)
            print(f"Simplified to {len(tones)} tones")
        
        print("\nTone Sequence:")
        print("  Freq (Hz)  Duration (ms)  Amplitude")
        print("  " + "-"*40)
        for freq, amp, dur in tones:
            print(f"  {freq:>8}   {int(dur*1000):>12}   {amp:>8.3f}")
        
        # Generate C++ code
        description = f"{len(tones)} tones, {duration:.2f}s total"
        code = generate_tone_function(tones, function_name, description)
        
        # Write header file
        with open(header_file, 'w') as f:
            f.write(f"// Generated from {wav_file}\n")
            f.write(f"// {description}\n")
            f.write(f"// Original duration: {duration:.2f}s\n\n")
            f.write("#ifndef " + function_name.upper() + "_TONES_H\n")
            f.write("#define " + function_name.upper() + "_TONES_H\n\n")
            f.write("#include <AudioControl.h>\n\n")
            f.write(code)
            f.write("\n#endif\n")
        
        print(f"\n{'='*60}")
        print(f"✓ Tone sequence generated: {header_file}")
        print(f"✓ Function: play_{function_name}_tones()")
        print(f"{'='*60}\n")
        
        # Show usage example
        print("Usage in your code:")
        print(f"  #include \"{header_file}\"")
        print(f"  play_{function_name}_tones();")
        print()
        
        return True
        
    except Exception as e:
        print(f"\n❌ Error: {e}")
        import traceback
        traceback.print_exc()
        return False

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description='Convert WAV audio to tone sequences for ESP32',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Analyze sparkle.wav and generate sparkle_tones.h
  python wav_to_tones.py assets/audio/sparkle.wav lib/AudioControl/sounds/sparkle_tones.h sparkle

  # Process all audio files
  python wav_to_tones.py assets/audio/chimes.wav lib/AudioControl/sounds/chimes_tones.h chimes
        """
    )
    
    parser.add_argument('input_wav', help='Input WAV file')
    parser.add_argument('output_header', help='Output .h file')
    parser.add_argument('function_name', help='Function name (e.g., "sparkle" → play_sparkle_tones)')
    
    args = parser.parse_args()
    
    # Check dependencies
    try:
        import numpy as np
        from scipy import signal, fft
    except ImportError:
        print("❌ Error: This script requires numpy and scipy")
        print("Install with: pip install numpy scipy")
        sys.exit(1)
    
    if wav_to_tones(args.input_wav, args.output_header, args.function_name):
        print("✓ Conversion successful!")
        sys.exit(0)
    else:
        print("❌ Conversion failed!")
        sys.exit(1)
