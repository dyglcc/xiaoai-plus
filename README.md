# xiaoai-plus

`xiaoai-plus` 是一个基于 [idootop/open-xiaoai](https://github.com/idootop/open-xiaoai) 修改的项目，用于在小爱音箱上实现本地唤醒和实时语音对话。

## 介绍

- 面向小爱音箱设备侧运行。
- 基于 `open-xiaoai` 做了工程化调整和部署流程整理。
- 主程序运行在音箱本机，不需要额外自建中转服务器。
- 对话能力通过火山引擎 Realtime Dialogue 接口提供。

## 特色

- 对话流程接近豆包语音交互方式，支持唤醒后连续对话和打断。
- 音箱内置运行，不需要额外服务器。
- 支持自定义关键词唤醒（修改 `assets/keywords.txt`）。
- 内置本地唤醒与静音过滤，减少无效触发。
- 集成回声消除、降噪、增益控制，提升设备侧通话质量。
- 提供 Docker（Dock）交叉编译和打包命令。

## 使用指南

### 1. 准备条件

- 开发机安装并可使用 Docker（用于交叉编译）。
- 设备可用 `arecord` / `aplay`，并具备网络访问能力。
- 准备好火山引擎 Realtime 对话所需密钥。

### 2. 配置

在项目根目录执行：

```bash
cp config.ini.example config.ini
```

然后编辑 `config.ini`，至少填写：

- `[realtime].app_id`
- `[realtime].access_key`
- `[realtime].app_key`

### 3. 交叉编译（Docker / Dock）

在项目根目录执行：

```bash
# 1) 首次构建工具链镜像
make -C toolchain build

# 2) 首次或依赖升级时构建静态库
make -C toolchain build-sherpa-static

# 3) 编译 armv7 可执行文件
make -C toolchain compile

# 4) 打包发布目录
make -C toolchain dist
```

### 4. 运行

将 `dist/xiaoai-plus-armhf/` 放到设备后执行：

```bash
cd dist/xiaoai-plus-armhf
./run.sh
```

## 开发指南

### 目录说明

- `src/app/`: 应用主流程与会话状态管理
- `src/wakeup/`: 本地唤醒、关键词匹配与触发门控
- `src/realtime/`: WebSocket 协议与对话事件处理
- `src/audio/`: 录音、播放、AEC/NS/AGC 音频处理
- `toolchain/`: Docker 交叉编译和打包脚本
- `assets/`: 模型、唤醒词和资源文件

### 常用命令

```bash
# 重新编译
make -C toolchain compile

# 编译并打包
make -C toolchain dist
```

### 自定义关键词

- 编辑 `assets/keywords.txt`。
- 重新打包并在设备上测试唤醒效果。

### 排查建议

- 启动失败：先检查 `config.ini` 三个密钥是否正确。
- 无声或录音异常：先检查设备侧 `arecord/aplay` 是否可用。
- 连接失败：检查设备网络和 Realtime 服务可达性。

## 致谢

本项目基于 [idootop/open-xiaoai](https://github.com/idootop/open-xiaoai) 修改。
