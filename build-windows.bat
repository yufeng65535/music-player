@echo off
REM MusicPlayer Windows 原生构建脚本
REM 在 Windows 上运行，需要已安装:
REM   1. Visual Studio 2019/2022 (C++ 工作负载)
REM   2. Qt 6.x (MinGW 或 MSVC)
REM   3. TagLib (通过 vcpkg 或手动安装)
REM
REM 用法: 在 "x64 Native Tools Command Prompt" 中运行 build-windows.bat

setlocal EnableDelayedExpansion

echo === MusicPlayer Windows 构建 ===

REM ── 1. 检查依赖 ─────────────────────────────────────────────
where cmake >nul 2>&1 || (echo 错误: 未找到 cmake && exit /b 1)
where cl >nul 2>&1 || (echo 错误: 请在 Visual Studio 开发者命令提示符中运行 && exit /b 1)

REM ── 2. 配置 ──────────────────────────────────────────────────
echo [1/3] 配置 CMake...
if exist build rmdir /s /q build
mkdir build && cd build

REM 设置 Qt 路径 (根据你的安装位置修改)
REM set QT_DIR=C:\Qt\6.7.2\msvc2019_64
REM set CMAKE_PREFIX_PATH=%QT_DIR%

REM 如果使用 vcpkg 安装 TagLib:
REM set VCPKG_ROOT=C:\vcpkg
REM set CMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake

cmake .. -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022" -A x64

REM ── 3. 编译 ──────────────────────────────────────────────────
echo [2/3] 编译...
cmake --build . --config Release -- /m

REM ── 4. 部署 ──────────────────────────────────────────────────
echo [3/3] 部署...
cd ..
if exist release rmdir /s /q release
mkdir release
copy build\Release\music.exe release\

REM 使用 windeployqt 打包 Qt 依赖
REM windeployqt release\music.exe --release --no-compiler-runtime

echo.
echo === 构建完成 ===
echo 输出: release\music.exe
echo.
echo 运行: release\music.exe

endlocal
