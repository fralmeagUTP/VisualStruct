param(
    [string]$ExePath = ".\visualstruct.exe",
    [string]$OutputRoot = ".\artifacts\e2e_visual_matrix",
    [int]$Runs = 3
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

if ($Runs -lt 2) { $Runs = 2 }

if (-not (Test-Path $ExePath)) {
    throw "No existe el ejecutable: $ExePath"
}

New-Item -ItemType Directory -Path $OutputRoot -Force | Out-Null

$runReports = @()
$globalErrors = New-Object System.Collections.Generic.List[string]

for ($i = 1; $i -le $Runs; $i++) {
    $runDir = Join-Path $OutputRoot ("run_{0:D2}" -f $i)
    New-Item -ItemType Directory -Path $runDir -Force | Out-Null

    Write-Host "Ejecutando corrida E2E visual $i/$Runs ..."
    & powershell -ExecutionPolicy Bypass -File ".\scripts\run_visual_e2e.ps1" -ExePath $ExePath -OutputDir $runDir
    if ($LASTEXITCODE -ne 0) {
        $globalErrors.Add("Corrida ${i}: runner visual devolvio codigo $LASTEXITCODE")
    }

    $reportPath = Join-Path $runDir "report.json"
    if (-not (Test-Path $reportPath)) {
        $globalErrors.Add("Corrida ${i}: no se encontro report.json")
        continue
    }

    $json = Get-Content $reportPath -Raw | ConvertFrom-Json
    $runReports += [pscustomobject]@{
        run = $i
        status = $json.status
        captures = $json.captures
        expected = $json.expected_stages
        report = $reportPath
        images = $json.images
    }
}

# Validacion de variacion entre corridas:
# al menos una escena de grafo debe cambiar de hash entre runs.
$hashPerRun = @{}
foreach ($r in $runReports) {
    $dir = Split-Path -Parent $r.report
    $imgs = @(Get-ChildItem -Path $dir -Filter "*grafo_*.png" | Sort-Object Name)
    $hashes = @()
    foreach ($img in $imgs) {
        $h = Get-FileHash -Path $img.FullName -Algorithm MD5
        $hashes += $h.Hash
    }
    $hashPerRun[$r.run] = ($hashes -join "|")
}

$distinctHashes = @($hashPerRun.Values | Select-Object -Unique)
if ($distinctHashes.Count -lt 2) {
    $globalErrors.Add("No se detecto variacion visual entre corridas de escenas de grafo.")
}

$passRuns = @($runReports | Where-Object { $_.status -eq "PASS" }).Count
$overall = if ($globalErrors.Count -eq 0 -and $passRuns -eq $runReports.Count) { "PASS" } else { "FAIL" }

$summary = [pscustomobject]@{
    date = (Get-Date).ToString("yyyy-MM-dd HH:mm:ss")
    runs = $Runs
    pass_runs = $passRuns
    status = $overall
    errors = $globalErrors
    report_paths = $runReports.report
}

$summary | ConvertTo-Json -Depth 6 | Out-File -FilePath (Join-Path $OutputRoot "matrix_report.json") -Encoding utf8

$dateTag = Get-Date -Format "yyyy-MM-dd"
$mdPath = Join-Path ".\docs" "informe-e2e-visual-matriz-$dateTag.md"
$lines = New-Object System.Collections.Generic.List[string]
$lines.Add("# Informe E2E Visual Matriz - $dateTag")
$lines.Add("")
$lines.Add("## Resumen")
$lines.Add("- Corridas: $Runs")
$lines.Add("- Corridas PASS: $passRuns")
$lines.Add("- Estado global: **$overall**")
$lines.Add("- Salida: ``$((Resolve-Path $OutputRoot).Path)``")
$lines.Add("")
$lines.Add("## Detalle por corrida")
foreach ($r in $runReports) {
    $lines.Add("- Run $($r.run): $($r.status) | Capturas $($r.captures)/$($r.expected) | Reporte: ``$($r.report)``")
}
$lines.Add("")
$lines.Add("## Validaciones de matriz")
$lines.Add("- Todas las corridas deben quedar en PASS.")
$lines.Add("- Debe existir variacion visual entre corridas en escenas de grafo (aleatoriedad).")
$lines.Add("")
$lines.Add("## Errores")
if ($globalErrors.Count -eq 0) {
    $lines.Add("- Ninguno")
} else {
    foreach ($e in $globalErrors) { $lines.Add("- $e") }
}

$lines | Out-File -FilePath $mdPath -Encoding utf8

Write-Host "Matriz E2E visual finalizada. Estado global: $overall"
Write-Host "Reporte matriz JSON: $(Join-Path $OutputRoot 'matrix_report.json')"
Write-Host "Reporte matriz MD: $mdPath"

if ($overall -ne "PASS") {
    exit 1
}
