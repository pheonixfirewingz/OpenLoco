#!/usr/bin/env pwsh
# OpenLoco WebAssembly Web Server
# This script starts a simple HTTP server to serve the OpenLoco web build

param(
    [int]$Port = 8080,
    [switch]$OpenBrowser = $true
)

# Get the project root directory (parent of scripts directory)
$ScriptDir = Split-Path -Path $PSScriptRoot -Parent
$BuildDir = Join-Path -Path $ScriptDir -ChildPath "build/emscripten"
$WebDir = Join-Path -Path $ScriptDir -ChildPath "distribution/web"

# Check if the build directory exists
if (-not (Test-Path $BuildDir)) {
    Write-Error "Build directory not found at $BuildDir"
    Write-Host "Please build the project first using: cmake --build --preset emscripten" -ForegroundColor Yellow
    exit 1
}

# Check if OpenLoco.js exists
$OpenLocoJs = Join-Path -Path $BuildDir -ChildPath "OpenLoco.js"
if (-not (Test-Path $OpenLocoJs)) {
    Write-Error "OpenLoco.js not found in $BuildDir"
    Write-Host "Please build the project first using: cmake --build --preset emscripten" -ForegroundColor Yellow
    exit 1
}

# Copy web files (index.html, favicon.ico) to build directory if they exist
$IndexHtml = Join-Path -Path $WebDir -ChildPath "index.html"
$FaviconIco = Join-Path -Path $WebDir -ChildPath "favicon.ico"

if (Test-Path $IndexHtml) {
    Copy-Item -Path $IndexHtml -Destination $BuildDir -Force
    Write-Host "Copied index.html to build directory" -ForegroundColor Gray
}
if (Test-Path $FaviconIco) {
    Copy-Item -Path $FaviconIco -Destination $BuildDir -Force
    Write-Host "Copied favicon.ico to build directory" -ForegroundColor Gray
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  OpenLoco WebAssembly Server" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Serving files from: $BuildDir" -ForegroundColor Green
Write-Host "Server URL: http://localhost:$Port" -ForegroundColor Green
Write-Host ""
Write-Host "Note: The game requires Locomotion data files to run." -ForegroundColor Yellow
Write-Host "Place your Locomotion installation files in the appropriate directory" -ForegroundColor Yellow
Write-Host "or configure the path using command line options." -ForegroundColor Yellow
Write-Host ""
Write-Host "Press Ctrl+C to stop the server" -ForegroundColor Yellow
Write-Host ""

# Start the web server
try {
    if ($OpenBrowser) {
        Start-Process "http://localhost:$Port"
    }

    # Use Python's built-in HTTP server
    Push-Location $BuildDir
    python -m http.server $Port
    Pop-Location
}
catch {
    Write-Error "Failed to start server: $_"
    exit 1
}
