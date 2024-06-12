$ErrorActionPreference = "Stop"

# Needed for Invoke-WebRequest to work via CI.
$progressPreference = "silentlyContinue"

Invoke-WebRequest -URI "https://github.com/ethereum/evmone/releases/download/v0.11.0/evmone-0.11.0-windows-amd64.zip" -OutFile "evmone.zip"
tar -xf evmone.zip "bin/evmone.dll"
mv bin/evmone.dll deps/
