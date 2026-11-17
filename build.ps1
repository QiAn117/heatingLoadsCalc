# ==================================================================
#  build.ps1 —— 一键编译「房间冬季采暖负荷计算系统」
#
#  用法：
#    .\build.ps1                  # 用 gcc 编译到 output\app.exe（带全部警告）
#    .\build.ps1 -Compiler clang  # 换 clang
#    .\build.ps1 -DebugBuild      # 关闭优化、加入调试信息
#    .\build.ps1 -Strict          # 额外检查 -Wconversion -Wpedantic（更严格）
#
#  说明：不要用 gcc *.c 通配编译——目录下若还有别的 .c（例如旧的 debug.c），
#        会因重复定义 main()/全局变量而链接失败。源文件清单在 $sources 中显式列出。
# ==================================================================
param(
    [string]$Compiler = "gcc",
    [switch]$DebugBuild,
    [switch]$Strict,
    [switch]$Clean
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $root

$sources = @(
    "main.c",
    "EnvelopeIO.c",
    "WeatherIO.c",
    "KCorrect.c",
    "HeatLoss.c",
    "Infiltration.c",
    "Equilibrium.c",
    "SumLoad.c",
    "Sensitivity.c",
    "Report.c"
)

$outDir = Join-Path $root "output"
$outExe = Join-Path $outDir "app.exe"

if ($Clean) {
    Remove-Item $outExe -Force -ErrorAction SilentlyContinue
    Write-Host "[clean] 已删除 $outExe"
    return
}

if (-not (Get-Command $Compiler -ErrorAction SilentlyContinue)) {
    Write-Error "找不到编译器 '$Compiler'，请先安装 MinGW-w64 或改用 -Compiler 指定。"
}

New-Item -ItemType Directory -Force -Path $outDir | Out-Null

$flags = @("-std=c11", "-Wall", "-Wextra", "-Wshadow")
if ($Strict) { $flags += @("-Wconversion", "-Wpedantic") }
if ($DebugBuild) { $flags += @("-g", "-O0") } else { $flags += @("-O2") }

Write-Host "[build] $Compiler $($flags -join ' ')"
& $Compiler @flags -o $outExe @sources
if ($LASTEXITCODE -ne 0) { Write-Error "编译失败（exit $LASTEXITCODE）" }

Write-Host "[ok] 已生成 $outExe"
Write-Host "[提示] app.exe 读取「当前目录」下的 envelope.txt / weather.txt，"
Write-Host "       请把数据文件拷到 exe 同目录再运行。"
