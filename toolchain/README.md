# xiaoai-plus toolchain

ARM (armv7) 交叉编译工具链，所有依赖静态链接。

## 构建流程

```bash
# 在项目根目录执行
make -C toolchain build
make -C toolchain build-sherpa-static
make -C toolchain compile
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
| CI 发布 | 由 CI 负责打包与发布发行产物 |

仅依赖设备系统库：libstdc++, libc, libm, libgcc_s。
