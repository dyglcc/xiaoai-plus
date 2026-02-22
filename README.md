# xiaoai-plus

[![CI](https://github.com/kslr/xiaoai-plus/actions/workflows/ci.yml/badge.svg)](https://github.com/OWNER/xiaoai-plus/actions/workflows/ci.yml)
[![Release](https://img.shields.io/github/v/release/kslr/xiaoai-plus?include_prereleases)](https://github.com/OWNER/xiaoai-plus/releases)
[![Downloads](https://img.shields.io/github/downloads/kslr/xiaoai-plus/total)](https://github.com/OWNER/xiaoai-plus/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](#%E8%AE%B8%E5%8F%AF-license)
[![Stars](https://img.shields.io/github/stars/kslr/xiaoai-plus?style=social)](https://github.com/kslr/xiaoai-plus)

在小爱音箱上获得与豆包近乎一致的端侧实时语音对话体验


## 支持设备

| 型号 | 设备代号 |
| --- | --- |
| Xiaomi 智能音箱 Pro | OH2P |

## 特性

- **实时对话体验**：与移动端豆包一致，支持实时互动、音色定义、连续对话与随时打断。
- **双助手共存**：小爱同学与豆包同学可同时运行，拥有独立的唤醒词。
- **纯本地运行**：程序完全在音箱本机执行，无需搭建外部中转服务器。
- **自定义唤醒**：支持根据需求自定义关键词进行语音唤醒。
- **远场优化**：集成 AEC（回声消除）、NS（降噪）、AGC（增益），大幅提升远场唤醒与对话的准确率。

## 快速开始

> [!IMPORTANT]
> 本教程仅适用于 **Xiaomi 智能音箱 Pro（OH2P）**，**其他型号**的小爱音箱请勿直接使用！🚨

1. 刷机更新小爱音箱补丁固件，开启并 SSH 连接到小爱音箱 👉 [教程](https://github.com/idootop/open-xiaoai/blob/main/docs/flash.md))
2. 开通「豆包端到端实时语音大模型」👉 [火山引擎]([packages/client-rust/README.md](https://www.volcengine.com/product/realtime-voice-model))
3. 执行安装脚本（会自动下载并安装最新 release 到 `/data/xiaoai-plus`）
   ```sh
   curl -sSfL https://raw.githubusercontent.com/kslr/xiaoai-plus/main/install.sh | sh
   ```
4. 设置开机自启动（下载 `boot.sh` 到 `/data/init.sh`）
   ```sh
   curl -L -o /data/init.sh https://raw.githubusercontent.com/kslr/xiaoai-plus/main/boot.sh
   chmod +x /data/init.sh
   ```
5. 编辑配置并手动启动一次验证
   ```sh
   cp /data/xiaoai-plus/config.ini.example /data/xiaoai-plus/config.ini
   vi /data/xiaoai-plus/config.ini
   /data/xiaoai-plus/xiaoai_plus_speaker -c /data/xiaoai-plus/config.ini
   ```
6. 重启设备验证开机自启动
   ```sh
   reboot
   ```


### 自定义关键词

- 编辑 assets/keywords.txt
- 重新打包并在设备上测试唤醒效果

## 免责声明

- 本项目仅供学习与研究，请确保在合法合规前提下使用。
- 项目与小米、火山引擎/字节跳动无从属关系，品牌与商标归属其各自权利人。

## 致谢

本项目大量参考 https://github.com/idootop/open-xiaoai 研究和开发。
