# ==================================================================
#  run_tests.ps1 —— 回归测试：逐套跑 tests\ 下的数据，核对关键输出
#
#  用法：
#    .\run_tests.ps1           # 用 output\app.exe 跑全部用例
#    .\run_tests.ps1 -Build    # 先编译再跑
#
#  每个用例都在临时目录里运行（程序读的是"当前目录"下的
#  envelope.txt / weather.txt），跑完自动清理。
# ==================================================================
[CmdletBinding()]
param(
    [string]$Exe = "",
    [switch]$Build
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $root

if (-not $Exe) { $Exe = Join-Path $root "output\app.exe" }
if ($Build) {
    & (Join-Path $root "build.ps1")
    if ($LASTEXITCODE -ne 0) { exit 1 }
}
if (-not (Test-Path $Exe)) {
    Write-Error "找不到 $Exe，请先运行 .\build.ps1 或加 -Build 参数。"
}

# 菜单序列：1 读围护 → 2 读气象 → 3 修正 → 5 平衡迭代 → 6 汇总（回车用默认 18C）
#           → 7 敏感性实验 → 8 报表 → 0 退出
$keys = "1`n2`n3`n5`n6`n`n7`n8`n0`n"

$cases = @(
    @{ name = "01_clean"
       expect = @("已读入围护面 7 个（有效 7 个，警告 0 条）",
                  "总负荷 1862.01 W",
                  "单位面积指标 103.45 W/m2",
                  "平衡室温约 32.66 C",
                  "报表已写入 heat_load_report.txt") },

    @{ name = "02_anomaly"
       expect = @("面积 126.00 超出范围",
                  "朝向代码非法 'X'",
                  "有效 5 个，警告 2 条",
                  "疑似丢失负号，已自动取负",
                  "总负荷 1481.39 W",
                  "平衡室温约 45.94 C") },

    @{ name = "03_harbin"
       expect = @("室外计算温度 tw = -24.2 C",
                  "总负荷 2182.23 W",
                  "单位面积指标 121.24 W/m2",
                  "平衡室温约 33.81 C") },

    @{ name = "04_midroom"
       expect = @("已读入围护面 8 个（有效 8 个，警告 0 条）",
                  "总负荷 1819.92 W",
                  "单位面积指标 91.00 W/m2",
                  "平衡室温约 48.46 C") },

    @{ name = "05_bad_format"
       expect = @("有效 2 个，警告 8 条",
                  "室外计算温度为 0 C",
                  "未知关键字，已忽略：未定义参数",
                  "总负荷 285.63 W",
                  "平衡室温约 163.85 C") },

    @{ name = "06_boundary"
       expect = @("总负荷 6277.79 W",
                  "单位面积指标 348.77 W/m2",
                  "平衡室温约 -13.11 C",
                  "供热能力不足") },

    @{ name = "07_all_invalid"
       expect = @("有效 0 个，警告 3 条",
                  "没有有效围护面，房间面积与负荷指标无法计算",
                  "围护总传热能力 S = 0",
                  "5 档供热能力未收敛") }
)

$tmpRoot = Join-Path ([IO.Path]::GetTempPath()) ("a4tests_" + [Guid]::NewGuid().ToString("N"))
[IO.Directory]::CreateDirectory($tmpRoot) | Out-Null

$pass = 0
$fail = 0
$results = @()

try {
    foreach ($case in $cases) {
        $src = Join-Path $root ("tests\" + $case.name)
        if (-not (Test-Path $src)) {
            $results += [pscustomobject]@{ 用例 = $case.name; 结果 = "SKIP"; 说明 = "缺少数据目录" }
            continue
        }

        $dir = Join-Path $tmpRoot $case.name
        [IO.Directory]::CreateDirectory($dir) | Out-Null
        Copy-Item (Join-Path $src "*.txt") $dir -Force

        Push-Location $dir
        $out = $keys | & $Exe 2>&1 | Out-String
        $code = $LASTEXITCODE
        Pop-Location

        $missing = @()
        foreach ($pattern in $case.expect) {
            if ($out -notmatch [regex]::Escape($pattern)) { $missing += $pattern }
        }
        # 数值健壮性：输出里不应出现 inf/nan 或科学计数法的天文数字
        if ($out -match "inf|nan|e\+[0-9]") { $missing += "输出含 inf/nan/溢出数值" }
        if ($code -ne 0) { $missing += "退出码 $code" }

        $report = Join-Path $dir "heat_load_report.txt"
        if (-not (Test-Path $report)) { $missing += "未生成报表" }

        if ($missing.Count -eq 0) {
            $pass++
            $results += [pscustomobject]@{ 用例 = $case.name; 结果 = "PASS"; 说明 = "" }
        } else {
            $fail++
            $results += [pscustomobject]@{ 用例 = $case.name; 结果 = "FAIL"; 说明 = ($missing -join "; ") }
        }
    }
} finally {
    Remove-Item $tmpRoot -Recurse -Force -ErrorAction SilentlyContinue
}

$results | Format-Table -AutoSize | Out-String | Write-Host
Write-Host ("合计：PASS {0} / FAIL {1}" -f $pass, $fail)
if ($fail -gt 0) { exit 1 }
