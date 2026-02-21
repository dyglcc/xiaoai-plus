# xiaoai-plus toolchain

ARM (armv7) 交叉编译工具链，所有依赖静态链接。

## 构建流程

```bash
cd toolchain/

# 1. 构建 Docker 工具链镜像（首次）
make build

# 2. 编译 sherpa-onnx 静态库（首次或升级时）
make build-sherpa-static

# 3. 编译主程序
make compile

# 4. 编译 + 打包发行版
make dist
```

首次使用请先创建真实配置：

```bash
cp config.ini.example config.ini
# 然后填写 realtime 的密钥
```

## Make 目标

| 命令 | 说明 |
|------|------|
| `make build` | 构建 Docker 工具链镜像 |
| `make build-sherpa-static` | 从源码编译 sherpa-onnx + onnxruntime 静态库 |
| `make compile` | 交叉编译 `xiaoai_plus_speaker`（全静态链接） |
| `make dist` | 编译 + 打包到 `dist/xiaoai-plus-armhf/` |
| `make shell` | 进入工具链 Docker 交互式 shell |

## 产出

```
dist/xiaoai-plus-armhf/
├── bin/xiaoai_plus_speaker   # ARM 可执行文件（~16M, stripped）
├── assets/                   # 模型文件
├── config.ini.example
├── config.ini
├── run.sh
└── manifest.txt
```

仅依赖设备系统库：libstdc++, libc, libm, libgcc_s。
