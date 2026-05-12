# ArkZero Instrument Test 运行脚本
# 用于在设备上运行 Instrument Test（ohosTest）

# 配置
$env:DEVECO_SDK_HOME = "C:\Program Files\Huawei\DevEco Studio\sdk"
$HVIGOR = "C:\Program Files\Huawei\DevEco Studio\tools\node\node.exe"
$HVIGOR_SCRIPT = "C:\Program Files\Huawei\DevEco Studio\tools\hvigor\bin\hvigorw.js"
$PROJECT_ROOT = "C:\learn\ArkZero"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " ArkZero Instrument Test Runner" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 步骤 1: 检查设备连接
Write-Host "[1/5] 检查设备连接..." -ForegroundColor Yellow
$devices = hdc list targets
if ($devices -match "127.0.0.1") {
    Write-Host "✓ 设备已连接: 127.0.0.1:5555" -ForegroundColor Green
} else {
    Write-Host "✗ 未检测到设备" -ForegroundColor Red
    Write-Host "请启动 HarmonyOS 模拟器或连接真机" -ForegroundColor Red
    exit 1
}
Write-Host ""

# 步骤 2: 清理构建
Write-Host "[2/5] 清理构建..." -ForegroundColor Yellow
Set-Location $PROJECT_ROOT
& $HVIGOR $HVIGOR_SCRIPT --mode module -p module=entry -p product=default clean
if ($LASTEXITCODE -ne 0) {
    Write-Host "✗ 清理失败" -ForegroundColor Red
    exit 1
}
Write-Host "✓ 清理完成" -ForegroundColor Green
Write-Host ""

# 步骤 3: 构建 Instrument Test 包
Write-Host "[3/5] 构建 Instrument Test 包..." -ForegroundColor Yellow
& $HVIGOR $HVIGOR_SCRIPT --mode module -p module=entry -p product=default test
if ($LASTEXITCODE -ne 0) {
    Write-Host "✗ 构建失败" -ForegroundColor Red
    Write-Host "查看错误日志..." -ForegroundColor Red
    exit 1
}
Write-Host "✓ 构建完成" -ForegroundColor Green
Write-Host ""

# 步骤 4: 查找并安装测试包
Write-Host "[4/5] 安装测试应用到设备..." -ForegroundColor Yellow
$hapFile = Get-ChildItem "$PROJECT_ROOT\entry\build" -Recurse -Filter "*.hap" | 
           Where-Object { $_.FullName -match "entry-default-unsigned" } |
           Select-Object -First 1

if (-not $hapFile) {
    Write-Host "✗ 未找到测试包 .hap 文件" -ForegroundColor Red
    exit 1
}

Write-Host "安装包: $($hapFile.FullName)" -ForegroundColor Gray
hdc app install $hapFile.FullName
if ($LASTEXITCODE -ne 0) {
    Write-Host "✗ 安装失败" -ForegroundColor Red
    exit 1
}
Write-Host "✓ 安装完成" -ForegroundColor Green
Write-Host ""

# 步骤 5: 运行测试
Write-Host "[5/5] 运行 Instrument Test..." -ForegroundColor Yellow
Write-Host "启动测试 Activity..." -ForegroundColor Gray
hdc shell aa start -a TestAbility -b com.example.arkzero.entry_test
Write-Host ""

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " 测试已启动！" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "查看测试日志：" -ForegroundColor Yellow
Write-Host "  hdc hilog | Select-String 'hypium'" -ForegroundColor Gray
Write-Host ""
Write-Host "查看测试报告：" -ForegroundColor Yellow
Write-Host "  hdc file recv /data/app/el2/100/base/com.example.arkzero.entry_test/files/test_report.xml ./test_report.xml" -ForegroundColor Gray
Write-Host ""
