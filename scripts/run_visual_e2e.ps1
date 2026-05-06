param(
    [string]$ExePath = ".\visualstruct.exe",
    [string]$OutputDir = ".\artifacts\e2e_visual",
    [int]$TimeoutSec = 90
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Get-ImageStats {
    param(
        [string]$Path,
        [int]$SampleStep = 4
    )

    Add-Type -AssemblyName System.Drawing
    $bmp = New-Object System.Drawing.Bitmap($Path)
    try {
        $width = $bmp.Width
        $height = $bmp.Height
        $dark = 0
        $bright = 0
        $samples = 0
        $colorBuckets = New-Object "System.Collections.Generic.HashSet[string]"

        for ($y = 0; $y -lt $height; $y += $SampleStep) {
            for ($x = 0; $x -lt $width; $x += $SampleStep) {
                $px = $bmp.GetPixel($x, $y)
                $samples++
                $avg = ($px.R + $px.G + $px.B) / 3.0
                if ($avg -lt 90) { $dark++ }
                if ($avg -gt 220) { $bright++ }
                $bucket = "{0:D2}-{1:D2}-{2:D2}" -f [int]($px.R / 16), [int]($px.G / 16), [int]($px.B / 16)
                [void]$colorBuckets.Add($bucket)
            }
        }

        [pscustomobject]@{
            width = $width
            height = $height
            samples = $samples
            dark_ratio = if ($samples -gt 0) { [math]::Round($dark / $samples, 4) } else { 0.0 }
            bright_ratio = if ($samples -gt 0) { [math]::Round($bright / $samples, 4) } else { 0.0 }
            color_bucket_count = $colorBuckets.Count
        }
    }
    finally {
        $bmp.Dispose()
    }
}

if (-not (Test-Path $ExePath)) {
    throw "No existe el ejecutable: $ExePath"
}

$exeDir = Split-Path -Parent (Resolve-Path $ExePath).Path
$rawShotsPattern = "*.png"

New-Item -ItemType Directory -Path $OutputDir -Force | Out-Null
Remove-Item -Path (Join-Path $OutputDir "*.png") -ErrorAction SilentlyContinue
Remove-Item -Path (Join-Path $OutputDir "manifest.txt") -ErrorAction SilentlyContinue
Remove-Item -Path (Join-Path $OutputDir "report.json") -ErrorAction SilentlyContinue
$existingRaw = @(Get-ChildItem -Path $exeDir -Filter $rawShotsPattern -ErrorAction SilentlyContinue |
    Where-Object { $_.Name -match "^\d\d_.+\.png$" })
foreach ($f in $existingRaw) {
    Remove-Item -Path $f.FullName -ErrorAction SilentlyContinue
}

$args = @("--e2e-visual", "--e2e-out=$OutputDir")
& $ExePath @args
if ($LASTEXITCODE -ne 0) {
    throw "La app finalizo con codigo $LASTEXITCODE en modo E2E visual."
}

$manifest = Join-Path $OutputDir "manifest.txt"
if (-not (Test-Path $manifest)) {
    throw "No se genero manifest.txt en $OutputDir"
}

$rawShots = @(Get-ChildItem -Path $exeDir -Filter $rawShotsPattern -ErrorAction SilentlyContinue |
    Where-Object { $_.Name -match "^\d\d_.+\.png$" } |
    Sort-Object Name)
foreach ($shot in $rawShots) {
    Move-Item -Path $shot.FullName -Destination (Join-Path $OutputDir $shot.Name) -Force
}

$manifestData = @{}
Get-Content $manifest | ForEach-Object {
    if ($_ -match "^\s*([^=]+)=(.*)\s*$") {
        $manifestData[$matches[1]] = $matches[2]
    }
}

$expectedStages = [int]($manifestData["stages"])
$captures = @(Get-ChildItem -Path $OutputDir -Filter "*.png" | Sort-Object Name)
$errors = New-Object System.Collections.Generic.List[string]
$details = @()

if ($captures.Count -ne $expectedStages) {
    $errors.Add("Cantidad de capturas inesperada: esperado=$expectedStages real=$($captures.Count)")
}

foreach ($img in $captures) {
    $stats = Get-ImageStats -Path $img.FullName
    $itemErrors = @()

    if ($stats.width -lt 1100 -or $stats.height -lt 680) {
        $itemErrors += "Resolucion baja ($($stats.width)x$($stats.height))"
    }
    if ($stats.color_bucket_count -lt 18) {
        $itemErrors += "Diversidad de color muy baja ($($stats.color_bucket_count) buckets)"
    }
    if ($img.Name -like "*grafo_*" -and $stats.dark_ratio -lt 0.01) {
        $itemErrors += "Poca tinta en escenas de grafo (dark_ratio=$($stats.dark_ratio))"
    }

    if ($itemErrors.Count -gt 0) {
        $errors.Add("$($img.Name): $($itemErrors -join '; ')")
    }

    $details += [pscustomobject]@{
        image = $img.Name
        width = $stats.width
        height = $stats.height
        dark_ratio = $stats.dark_ratio
        bright_ratio = $stats.bright_ratio
        color_bucket_count = $stats.color_bucket_count
        status = if ($itemErrors.Count -eq 0) { "PASS" } else { "FAIL" }
    }
}

$result = [pscustomobject]@{
    date = (Get-Date).ToString("yyyy-MM-dd HH:mm:ss")
    executable = (Resolve-Path $ExePath).Path
    output_dir = (Resolve-Path $OutputDir).Path
    expected_stages = $expectedStages
    captures = $captures.Count
    status = if ($errors.Count -eq 0) { "PASS" } else { "FAIL" }
    errors = $errors
    images = $details
}

$result | ConvertTo-Json -Depth 6 | Out-File -FilePath (Join-Path $OutputDir "report.json") -Encoding utf8

$dateTag = Get-Date -Format "yyyy-MM-dd"
$mdPath = Join-Path ".\docs" "informe-e2e-visual-$dateTag.md"
$lines = New-Object System.Collections.Generic.List[string]
$lines.Add("# Informe E2E Visual - $dateTag")
$lines.Add("")
$lines.Add("## Resultado")
$lines.Add("- Estado: **$($result.status)**")
$lines.Add("- Capturas: $($result.captures) / $($result.expected_stages)")
$lines.Add("- Ejecutable: ``$($result.executable)``")
$lines.Add("- Salida: ``$($result.output_dir)``")
$lines.Add("")
$lines.Add("## Validaciones automaticas")
$lines.Add("- Resolucion minima por captura: 1100x680")
$lines.Add("- Diversidad minima de color por captura")
$lines.Add("- Densidad minima de tinta para pantallas de grafo")
$lines.Add("")
$lines.Add("## Evidencia por captura")
foreach ($d in $details) {
    $lines.Add("- $($d.image): $($d.status) | $($d.width)x$($d.height) | dark=$($d.dark_ratio) | buckets=$($d.color_bucket_count)")
}
$lines.Add("")
if ($errors.Count -gt 0) {
    $lines.Add("## Errores detectados")
    foreach ($e in $errors) { $lines.Add("- $e") }
} else {
    $lines.Add("## Errores detectados")
    $lines.Add("- Ninguno")
}

$lines | Out-File -FilePath $mdPath -Encoding utf8

Write-Host "E2E visual finalizado. Estado: $($result.status)"
Write-Host "Reporte JSON: $(Join-Path $OutputDir 'report.json')"
Write-Host "Reporte MD: $mdPath"

if ($errors.Count -gt 0) {
    exit 1
}
