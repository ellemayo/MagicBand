# tools/convert_audio_for_dfplayer.ps1
param(
    [string]$SourceDir = ".\assets\audio",
    [string]$DestDir = ".\assets\mp3_for_sd"
)

# Ensure the destination directory exists and is empty
if (Test-Path $DestDir) {
    Write-Host "Clearing destination directory: $DestDir"
    Remove-Item "$DestDir\*" -Recurse -Force
} else {
    Write-Host "Creating destination directory: $DestDir"
    New-Item -ItemType Directory -Path $DestDir | Out-Null
}

# Find all audio files (WAV and MP3), excluding '-min' files
$audioFiles = @()
$audioFiles += Get-ChildItem -Path $SourceDir -Filter "*.wav" | Where-Object { $_.Name -notlike "*-min.wav" }
$audioFiles += Get-ChildItem -Path $SourceDir -Filter "*.mp3"

if ($audioFiles.Count -eq 0) {
    Write-Host "No audio files found in $SourceDir to convert."
    exit
}

Write-Host "Found $($audioFiles.Count) audio files to convert."

foreach ($file in $audioFiles) {
    $baseName = $file.BaseName
    $outputFile = Join-Path $DestDir "$($baseName).mp3"
    
    Write-Host "Converting $($file.Name) to MP3..."
    
    # Use ffmpeg to convert audio to MP3 with DFPlayer-optimized settings
    # -ar 44100: Set audio sample rate to 44.1kHz (DFPlayer compatible)
    # -ac 1: Convert to mono (reduces file size, DFPlayer works better with mono)
    # -b:a 128k: Set audio bitrate to 128kbps (Constant Bit Rate)
    # -codec:a libmp3lame: Use LAME encoder for better quality
    # -write_xing 0: Disable Xing header (can cause delays on some DFPlayer clones)
    # -id3v2_version 0: Disable ID3v2 tags (DFPlayer doesn't need them, reduces file size)
    # -y: Overwrite output file if it exists
    ffmpeg -i $file.FullName -ar 44100 -ac 1 -b:a 128k -codec:a libmp3lame -write_xing 0 -id3v2_version 0 -y $outputFile 2>&1 | Out-Null
    
    if ($LASTEXITCODE -ne 0) {
        Write-Error "ffmpeg failed to convert $($file.Name)."
    } else {
        Write-Host "Successfully converted $($file.Name)."
    }
}

Write-Host "Conversion complete. MP3 files are in $DestDir"
