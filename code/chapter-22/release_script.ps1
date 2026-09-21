# Simple release script for Windows PowerShell
$version = $args[0]
if (-not $version) { Write-Error "Usage: ./release_script.ps1 <version>"; exit 1 }
$dist = "dist"
if (-not (Test-Path $dist)) { New-Item -ItemType Directory -Path $dist | Out-Null }
Compress-Archive -Path "manuscript\*","code\*","README.md" -DestinationPath "$dist\c_book-$version.zip"
Write-Output "Created $dist\c_book-$version.zip"
