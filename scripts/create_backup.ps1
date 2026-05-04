param(
    [string]$VersionBase = "v0.0.2",
    [string]$Suffix = "",
    [string]$OutputDir = "backups"
)

$ErrorActionPreference = "Stop"

if (!(Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir | Out-Null
}

$timestamp = Get-Date -Format "yyyyMMdd-HHmmss"
$versionTag = $VersionBase
if ($Suffix -and $Suffix.Trim().Length -gt 0) {
    $versionTag = "$VersionBase-$Suffix"
}

$zipName = "visualstruct_${versionTag}_backup_${timestamp}.zip"
$zipPath = Join-Path $OutputDir $zipName

$items = Get-ChildItem -Force | Where-Object { $_.Name -notin @(".git", "backups") }
Compress-Archive -Path $items.FullName -DestinationPath $zipPath -Force

$head = (git rev-parse --short HEAD).Trim()
$dirtyCount = (git status --porcelain | Measure-Object).Count
$manifestPath = Join-Path $OutputDir "backup-manifest.md"

if (!(Test-Path $manifestPath)) {
    @"
# Manifest de respaldos

| Fecha | Archivo | Version base | Commit | Cambios sin commit |
|---|---|---|---|---|
"@ | Set-Content -Path $manifestPath -Encoding UTF8
}

"| $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss') | $zipName | $versionTag | $head | $dirtyCount |" |
    Add-Content -Path $manifestPath -Encoding UTF8

Write-Output "Backup creado: $zipPath"
