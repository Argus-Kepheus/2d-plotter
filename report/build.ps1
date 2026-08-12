<# Compile one report document with LuaLaTeX and clean auxiliary files. #>

[CmdletBinding()]
param(
    [string]$Document = 'relatorio',
    [switch]$Clean,
    [switch]$KeepTemp,
    [switch]$Open
)

$ErrorActionPreference = 'Stop'
$ReportDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ReportDir

$TexFile = "$Document.tex"
$PdfFile = "$Document.pdf"
$TempExtensions = @(
    '.aux', '.log', '.out', '.toc', '.fls', '.fdb_latexmk',
    '.synctex.gz', '.lof', '.lot', '.bbl', '.blg', '.run.xml', '.bcf',
    '.nav', '.snm', '.vrb', '.xdv'
)

function Remove-TempFiles {
    foreach ($extension in $TempExtensions) {
        $candidate = Join-Path $ReportDir "$Document$extension"
        if (Test-Path -LiteralPath $candidate -PathType Leaf) {
            Remove-Item -LiteralPath $candidate -Force
        }
    }
}

if ($Clean) {
    Remove-TempFiles
    return
}

if (-not (Test-Path -LiteralPath $TexFile -PathType Leaf)) {
    throw "Documento não encontrado: $TexFile"
}

$latexmk = Get-Command latexmk -ErrorAction SilentlyContinue
$lualatex = Get-Command lualatex -ErrorAction SilentlyContinue
if (-not $latexmk -and -not $lualatex) {
    throw 'Instale uma distribuição LaTeX que forneça latexmk ou lualatex.'
}

function Invoke-LuaLatex {
    for ($pass = 1; $pass -le 2; $pass++) {
        & lualatex -interaction=nonstopmode -halt-on-error $TexFile
        if ($LASTEXITCODE -ne 0) {
            throw "lualatex falhou na passagem $pass (código $LASTEXITCODE)."
        }
    }
}

try {
    if ($latexmk) {
        & latexmk -lualatex -interaction=nonstopmode -halt-on-error $TexFile
        if ($LASTEXITCODE -ne 0) {
            if (-not $lualatex) {
                throw "latexmk falhou (código $LASTEXITCODE)."
            }
            Write-Warning 'latexmk falhou; tentando duas passagens de lualatex.'
            Invoke-LuaLatex
        }
    } else {
        Invoke-LuaLatex
    }

    if (-not (Test-Path -LiteralPath $PdfFile -PathType Leaf)) {
        throw "A compilação não produziu $PdfFile."
    }
} catch {
    Write-Error "Falha de compilação: $_"
    Write-Host "Os temporários de $Document foram mantidos para diagnóstico."
    exit 1
}

if (-not $KeepTemp) {
    Remove-TempFiles
}

Write-Host "PDF gerado: $(Join-Path $ReportDir $PdfFile)"
if ($Open) {
    Invoke-Item $PdfFile
}
