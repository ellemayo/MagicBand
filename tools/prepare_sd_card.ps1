# tools/prepare_sd_card.ps1
param(
    [Parameter(Mandatory=$true)]
    [string]$DriveLetter,
    [string]$SourceDir = ".\assets\mp3_for_sd"
)

$rootDir = "${DriveLetter}:\\"

# Check if the drive is ready
if (-not (Test-Path $rootDir)) {
    Write-Error "Drive ${DriveLetter}: is not accessible. Please check the drive letter and ensure the SD card is inserted."
    exit
}

# Check if the source directory exists
if (-not (Test-Path $SourceDir)) {
    Write-Error "Source directory '$SourceDir' not found. Please run the audio conversion script first."
    exit
}

# Clear existing numbered MP3 files from the root directory
$existingFiles = Get-ChildItem -Path $rootDir -Filter "????_*.mp3"
if ($existingFiles) {
    Write-Host "Clearing existing numbered MP3 files from the root of drive ${DriveLetter}:"
    $existingFiles | Remove-Item -Force
}

# Get MP3 files and sort them alphabetically to ensure consistent numbering
$files = Get-ChildItem -Path $SourceDir -Filter "*.mp3" | Sort-Object Name

if ($files.Count -eq 0) {
    Write-Warning "No .mp3 files found in $SourceDir. The SD card will be empty."
    exit
}

# Copy and rename files with a 4-digit prefix
$i = 1
foreach ($file in $files) {
    # Format number with leading zeros (e.g., 1 -> 0001)
    $fileNumber = "{0:D4}" -f $i
    $newFileName = "${fileNumber}_$($file.Name)"
    $destinationPath = Join-Path $rootDir $newFileName
    
    Write-Host "Copying $($file.Name) to $destinationPath"
    Copy-Item -Path $file.FullName -Destination $destinationPath
    $i++
}

Write-Host "SD card preparation complete. Copied $($files.Count) files to the root of ${DriveLetter}:."
Write-Host "Please safely eject the SD card."
