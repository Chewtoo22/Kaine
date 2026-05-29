param(
    [string]$HostName = "127.0.0.1",
    [int]$Port = 8766
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $Root

$Python = Get-Command python -ErrorAction SilentlyContinue
if (-not $Python) {
    $Python = Get-Command py -ErrorAction SilentlyContinue
}
if (-not $Python) {
    throw "Python was not found on PATH. Install Python or add it to PATH, then run Start-Kaine.ps1 again."
}

$Url = "http://${HostName}:$Port"
Write-Host "Kaine local core starting at $Url"
Write-Host "Press Ctrl+C in this window to stop Kaine."
Start-Process $Url | Out-Null

& $Python.Source ".\kaine_server.py" --host $HostName --port $Port
