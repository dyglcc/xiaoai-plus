# xiaoai-plus

[![CI](https://github.com/kslr/xiaoai-plus/actions/workflows/ci.yml/badge.svg)](https://github.com/OWNER/xiaoai-plus/actions/workflows/ci.yml)
[![Release](https://img.shields.io/github/v/release/kslr/xiaoai-plus?include_prereleases)](https://github.com/OWNER/xiaoai-plus/releases)
[![Downloads](https://img.shields.io/github/downloads/kslr/xiaoai-plus/total)](https://github.com/OWNER/xiaoai-plus/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](#%E8%AE%B8%E5%8F%AF-license)
[![Stars](https://img.shields.io/github/stars/kslr/xiaoai-plus?style=social)](https://github.com/kslr/xiaoai-plus)

在小爱音箱上获得与豆包近乎一致的端侧实时语音对话体验（本机运行、独立唤醒、远场优化）。


## 支持设备

| 型号 | 设备代号 |
| --- | --- |
| Xiaomi 智能音箱 Pro | OH2P |

## 特性

- 实时对话体验：与移动端豆包一致（实时互动、音色定义、连续对话与可打断）
- 双助手共存：小爱同学与豆包同学可同时运行，独立唤醒词
- 纯本地运行：全部在音箱本机执行，无需外部服务器
- 自定义唤醒：支持自定义关键词唤醒
- 远场优化：AEC（回声消除）/ NS（降噪）/ AGC（增益），远场唤醒与对话体验更佳

## 使用指南


### 1) 为小爱音箱刷机以解锁 SSH 权限

https://github.com/kslr/open-xiaoai/blob/main/docs/flash.md

### 2) 开通「豆包端到端实时语音大模型」

访问火山引擎官网，开通对应语音模型能力：  
https://www.volcengine.com/product/realtime-voice-model

### 3) 部署到音箱

1. 前往本项目的 **Releases** 页面，下载与你设备匹配的文件。
2. 使用以下工具之一将文件上传到音箱的 `/data/xiaoai-plus` 目录：
   - **SCP 命令：**
     ```shell
     scp <local_file> <username>@<device_ip>:/data/xiaoai-plus
     ```
   - 或其他 SSH 文件管理工具
3. 初始化配置文件：
   - 复制示例配置：
     ```shell
     cp config.ini.example config.ini
     ```
   - 编辑 `config.ini`，填写你的 **火山引擎** 参数。

---

### 4) 启动程序

通过 SSH 登录音箱，并执行以下命令启动程序：

```shell
cd /data/xiaoai-plus
./xiaoai_plus_speaker -c config.ini
```

### 自定义关键词

- 编辑 assets/keywords.txt
- 重新打包并在设备上测试唤醒效果

## 免责声明

- 本项目仅供学习与研究，请确保在合法合规前提下使用。
- 项目与小米、火山引擎/字节跳动无从属关系，品牌与商标归属其各自权利人。

## 致谢

本项目基于 https://github.com/idootop/open-xiaoai 修改与扩展，感谢上游项目与社区的贡献。
