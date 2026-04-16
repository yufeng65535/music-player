#!/bin/bash
# macOS 原生构建脚本 — 在 Intel 或 Apple Silicon Mac 上运行
# 用法: ./build-macos.sh

set -e

echo "=== MusicPlayer macOS 构建 ==="

# ── 1. 安装依赖 ─────────────────────────────────────────────
echo "[1/5] 检查依赖..."
if ! command -v brew &>/dev/null; then
    echo "错误: 需要先安装 Homebrew: https://brew.sh"
    exit 1
fi

brew list qt@6 &>/dev/null || brew install qt@6
brew list taglib &>/dev/null || brew install taglib
command -v cmake &>/dev/null || brew install cmake

QT_PREFIX="$(brew --prefix qt@6)"
echo "Qt 路径: $QT_PREFIX"

# ── 2. 配置 ──────────────────────────────────────────────────
echo "[2/5] 配置 CMake..."
rm -rf build
mkdir build && cd build

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$QT_PREFIX" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET="12.0"

# ── 3. 编译 ──────────────────────────────────────────────────
echo "[3/5] 编译..."
make -j$(sysctl -n hw.ncpu)

if [ ! -f music ]; then
    echo "错误: 编译失败，未找到 music 可执行文件"
    exit 1
fi

# ── 4. 打包 .app ─────────────────────────────────────────────
echo "[4/5] 打包 .app..."
cd ..
mkdir -p release/MusicPlayer.app/Contents/{MacOS,Resources,Frameworks}

# 创建 Info.plist
cat > release/MusicPlayer.app/Contents/Info.plist <<'PLIST'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>music</string>
    <key>CFBundleIconFile</key>
    <string>icon.icns</string>
    <key>CFBundleIdentifier</key>
    <string>com.musicplayer.app</string>
    <key>CFBundleName</key>
    <string>MusicPlayer</string>
    <key>CFBundleDisplayName</key>
    <string>MusicPlayer</string>
    <key>CFBundleVersion</key>
    <string>0.1.0</string>
    <key>CFBundleShortVersionString</key>
    <string>0.1.0</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>LSMinimumSystemVersion</key>
    <string>12.0</string>
    <key>NSHumanReadableCopyright</key>
    <string>MusicPlayer</string>
</dict>
</plist>
PLIST

# 复制二进制
cp build/music release/MusicPlayer.app/Contents/MacOS/music
chmod +x release/MusicPlayer.app/Contents/MacOS/music

# 复制图标
cp resources/app-icon.svg release/MusicPlayer.app/Contents/Resources/

# ── 5. 使用 macdeployqt 打包 Qt 框架 ─────────────────────────
echo "[5/5] 运行 macdeployqt..."
"$QT_PREFIX/bin/macdeployqt" release/MusicPlayer.app -always-overwrite

# 检查签名 (Apple Silicon 必须)
ARCH="$(uname -m)"
if [ "$ARCH" = "arm64" ]; then
    echo "Apple Silicon 检测到，执行代码签名..."
    codesign --force --deep --sign - release/MusicPlayer.app
fi

echo ""
echo "=== 构建完成 ==="
echo "输出: release/MusicPlayer.app"
echo ""
echo "运行: open release/MusicPlayer.app"
echo ""
echo "创建 DMG:"
echo "  hdiutil create -volname MusicPlayer -srcfolder release/MusicPlayer.app -ov -format UDZO MusicPlayer.dmg"
