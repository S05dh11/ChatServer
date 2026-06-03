<div align="center">

# 🤖 AIChatServer

**基于 C++17 的 AI 智能聊天服务器**

支持多模型接入 · 流式对话（SSE） · 会话管理 · RESTful API

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/std/the-standard)
[![CMake](https://img.shields.io/badge/CMake-3.10+-green.svg)](https://cmake.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-Linux-orange.svg)](https://www.linux.org/)

[功能特性](#-功能特性) · [快速开始](#-快速开始) · [API 文档](#-api-接口) · [项目架构](#-项目架构)

</div>

---

## ✨ 功能特性

- 🌐 **多模型支持** — DeepSeek、豆包（Doubao）、通义千问（Qwen）、Ollama 本地模型，可同时接入多个模型
- ⚡ **流式响应** — 基于 SSE（Server-Sent Events）的实时流式输出，逐字显示 AI 回复
- 📦 **全量响应** — 支持一次性返回完整消息，适用于批处理场景
- 💬 **会话管理** — 创建、删除、查询会话及历史消息，基于 SQLite 持久化存储
- ⚙️ **配置灵活** — 支持命令行参数、配置文件、环境变量三种配置方式
- 🖥️ **静态资源托管** — 内置前端页面服务，开箱即用
- 🧩 **插件化 SDK** — 自研 AI ChatSDK，采用 Provider 模式，易于扩展新模型

## 🏗️ 项目架构

```
┌──────────────┐     HTTP/REST      ┌──────────────────┐
│              │  ───────────────►   │                  │
│   前端页面    │     SSE Stream     │   ChatServer     │
│  (HTML/CSS)  │  ◄───────────────  │  (cpp-httplib)   │
│              │                    │                  │
└──────────────┘                    └────────┬─────────┘
                                             │
                                    ┌────────▼─────────┐
                                    │    AI ChatSDK     │
                                    │  (libai_chat_sdk) │
                                    └────────┬─────────┘
                                             │
                    ┌────────────┬────────────┼────────────┬────────────┐
                    ▼            ▼            ▼            ▼            ▼
              ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐
              │ DeepSeek │ │  Doubao  │ │   Qwen   │ │  Ollama  │ │  更多... │
              │ Provider │ │ Provider │ │ Provider │ │ Provider │ │          │
              └──────────┘ └──────────┘ └──────────┘ └──────────┘ └──────────┘
```

## 📁 项目结构

```
ChatServer/
├── ChatServer/                    # 聊天服务器主程序
│   ├── CMakeLists.txt             # CMake 构建配置
│   ├── ChatServer.h               # 服务器类声明
│   ├── ChatServer.cpp             # 服务器类实现（路由、请求处理、SSE 流式推送）
│   ├── main.cpp                   # 程序入口（gflags 参数解析、配置校验）
│   ├── required/                  # 项目需求文档
│   │   ├── main需求.txt
│   │   └── Chat前端页面需求说明.txt
│   └── www/                       # 前端静态资源目录（需自行放置）
│
├── sdk/                           # 🧩 AI ChatSDK — 多模型接入 SDK（自研）
│   ├── CMakeLists.txt             # SDK 构建配置（编译为静态库 libai_chat_sdk.a）
│   ├── include/                   # 📌 SDK 头文件
│   │   ├── ChatSDK.h              #   SDK 主接口（对外统一入口）
│   │   ├── LLMProvider.h          #   LLM 提供者抽象基类
│   │   ├── LLMManager.h           #   LLM 管理器（模型注册、调度）
│   │   ├── SessionManager.h       #   会话管理器（会话生命周期）
│   │   ├── DataManager.h          #   数据持久化管理（SQLite）
│   │   ├── DeepSeekProvider.h     #   DeepSeek 模型接入实现
│   │   ├── DoubaoProvider.h       #   豆包（字节跳动）模型接入实现
│   │   ├── QwenProvider.h         #   通义千问（阿里）模型接入实现
│   │   ├── OllamaLLMProvider.h    #   Ollama 本地模型接入实现
│   │   ├── common.h               #   公共类型定义（消息、配置、会话等）
│   │   └── util/myLog.h           #   日志工具（基于 spdlog 封装）
│   └── src/                       # SDK 源文件
│       ├── ChatSDK.cpp
│       ├── LLMManager.cpp
│       ├── SessionManager.cpp
│       ├── DataManager.cpp
│       ├── DeepSeekProvider.cpp
│       ├── DoubaoProvider.cpp
│       ├── QwenProvider.cpp
│       ├── OllamaLLMProvider.cpp
│       └── util/myLog.cpp
│
└── test/                          # 🧪 测试代码
    ├── CMakeLists.txt
    ├── testLLM.cpp                # LLM 接口测试
    └── testSQLite/
        └── testSqlite3.cpp        # SQLite 数据库测试
```

## 🛠️ 技术栈

| 类别 | 技术 | 说明 |
|------|------|------|
| 语言标准 | C++17 | 使用 std::optional、std::filesystem 等现代特性 |
| HTTP 框架 | [cpp-httplib](https://github.com/yhirose/cpp-httplib) | 轻量级单文件 HTTP/HTTPS 服务器 |
| AI SDK | ai_chat_sdk（自研） | Provider 模式，支持流式/全量两种调用方式 |
| JSON | jsoncpp | 请求/响应序列化与反序列化 |
| 命令行 | gflags | 支持 CLI 参数 + 配置文件 + 环境变量 |
| 日志 | spdlog | 高性能异步日志，支持多级别输出 |
| 格式化 | fmt | 类型安全的现代化字符串格式化 |
| 数据库 | SQLite3 | 轻量级嵌入式数据库，会话与消息持久化 |
| SSL/TLS | OpenSSL | HTTPS 支持，模型 API 安全通信 |

## 🚀 快速开始

### 1. 安装依赖

Ubuntu / Debian：

```bash
sudo apt update
sudo apt install build-essential cmake libssl-dev libsqlite3-dev libjsoncpp-dev libspdlog-dev libfmt-dev libgflags-dev
```

CentOS / RHEL：

```bash
sudo yum install gcc-c++ cmake openssl-devel sqlite-devel jsoncpp-devel spdlog-devel fmt-devel gflags-devel
```

### 2. 编译安装 AI ChatSDK

```bash
cd sdk
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
# 安装内容：
#   静态库 → /usr/local/lib/libai_chat_sdk.a
#   头文件 → /usr/local/include/ai_chat_sdk/
```

### 3. 编译 ChatServer

```bash
cd ChatServer
mkdir build && cd build
cmake ..
make -j$(nproc)
```

编译成功后生成可执行文件 `build/AIChatServer`。

### 4. 配置 API Key

```bash
# 至少配置一个 API Key（二选一或全部配置）
export deepseek_apikey="sk-your-deepseek-api-key"
export doubao_apikey="your-doubao-api-key"
export qwen_apikey="sk-your-qwen-api-key"
```

> 💡 也可以将 API Key 写入 `ChatServer.conf` 配置文件。

### 5. 启动服务

```bash
./build/AIChatServer
```

看到以下输出即表示启动成功：

```
[INFO] AIChatServer 启动配置:
[INFO]   版本: 1.0.0
[INFO]   主机: 0.0.0.0
[INFO]   端口: 8080
[INFO] ChatServer 启动成功!
[INFO] 服务器地址: http://0.0.0.0:8080
```

### 6. 验证服务

```bash
# 获取可用模型列表
curl http://localhost:8080/api/models

# 创建会话
curl -X POST http://localhost:8080/api/session \
  -H "Content-Type: application/json" \
  -d '{"model": "deepseek-chat"}'

# 获取会话列表
curl http://localhost:8080/api/sessions
```

## ⚙️ 配置说明

### 命令行参数

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `--host` | `0.0.0.0` | 服务器绑定地址 |
| `--port` | `8080` | 服务器绑定端口 |
| `--log_level` | `INFO` | 日志级别（DEBUG/INFO/WARN/ERROR） |
| `--temperature` | `0.7` | 温度值，控制回复随机性（0.0 ~ 2.0） |
| `--max_tokens` | `2048` | 单次回复最大 token 数 |
| `--config_file` | `./ChatServer.conf` | 配置文件路径 |
| `--ollama_model_name` | — | Ollama 模型名称（如 deepseek-r1:1.5b） |
| `--ollama_model_desc` | — | Ollama 模型描述信息 |
| `--ollama_endpoint` | — | Ollama API 地址（如 http://localhost:11434） |

### 配置文件

在可执行文件同目录下创建 `ChatServer.conf`，程序启动时自动加载：

```bash
# ChatServer.conf 示例
--host=0.0.0.0
--port=8080
--temperature=0.7
--max_tokens=2048
--log_level=INFO
```

> **优先级**：命令行参数 > 配置文件 > 环境变量 > 默认值

### 更多启动示例

```bash
# 指定端口启动
./build/AIChatServer --port=9000

# 使用指定配置文件
./build/AIChatServer --config_file=/etc/chatserver/my_config.conf

# 使用 Ollama 本地模型（无需 API Key）
./build/AIChatServer \
  --ollama_model_name=deepseek-r1:1.5b \
  --ollama_model_desc="DeepSeek R1 1.5B 本地模型" \
  --ollama_endpoint=http://localhost:11434

# 查看帮助信息
./build/AIChatServer --help

# 查看版本号
./build/AIChatServer --version
```

## 📡 API 接口

所有接口均返回 JSON 格式数据，基础响应结构：

```json
{
  "success": true,
  "message": "描述信息",
  "data": {}
}
```

---

### 1️⃣ 创建会话

创建一个新的对话会话，需指定使用的模型。

```
POST /api/session
Content-Type: application/json
```

**请求体：**

```json
{
  "model": "deepseek-chat"
}
```

**响应：**

```json
{
  "success": true,
  "message": "create session success",
  "data": {
    "session_id": "a1b2c3d4-e5f6-7890",
    "model": "deepseek-chat"
  }
}
```

---

### 2️⃣ 获取会话列表

获取当前所有会话的概要信息。

```
GET /api/sessions
```

**响应：**

```json
{
  "success": true,
  "message": "get session lists success",
  "data": [
    {
      "id": "a1b2c3d4-e5f6-7890",
      "model": "deepseek-chat",
      "created_at": 1717400000,
      "updated_at": 1717400100,
      "message_count": 5,
      "first_user_message": "你好，请介绍一下你自己"
    }
  ]
}
```

---

### 3️⃣ 获取可用模型

获取服务器当前支持的所有模型列表。

```
GET /api/models
```

**响应：**

```json
{
  "success": true,
  "message": "get model lists success",
  "data": [
    { "name": "deepseek-chat", "desc": "DeepSeek 对话模型" },
    { "name": "doubao-pro", "desc": "豆包专业版" },
    { "name": "qwen3.6-plus", "desc": "通义千问 Plus" },
    { "name": "deepseek-r1:1.5b", "desc": "DeepSeek R1 本地模型" }
  ]
}
```

---

### 4️⃣ 删除会话

根据会话 ID 删除指定会话及其所有历史消息。

```
DELETE /api/session/{session_id}
```

**响应：**

```json
{
  "success": true,
  "message": "delete session success"
}
```

---

### 5️⃣ 获取历史消息

获取指定会话的所有历史消息记录。

```
GET /api/session/{session_id}/history
```

**响应：**

```json
{
  "success": true,
  "message": "get history messages success",
  "data": [
    {
      "id": "msg-001",
      "role": "user",
      "content": "你好",
      "timestamp": 1717400000
    },
    {
      "id": "msg-002",
      "role": "assistant",
      "content": "你好！有什么可以帮助你的吗？",
      "timestamp": 1717400001
    }
  ]
}
```

---

### 6️⃣ 发送消息（全量返回）

发送消息并等待 AI 完整回复后一次性返回。

```
POST /api/message
Content-Type: application/json
```

**请求体：**

```json
{
  "session_id": "a1b2c3d4-e5f6-7890",
  "message": "请用 C++ 写一个快速排序"
}
```

**响应：**

```json
{
  "success": true,
  "message": "send message success",
  "data": {
    "session_id": "a1b2c3d4-e5f6-7890",
    "response": "这是一个 C++ 快速排序的实现..."
  }
}
```

---

### 7️⃣ 发送消息（流式返回 SSE）

发送消息并以 SSE 流式格式逐字返回 AI 回复，适合实时展示场景。

```
POST /api/message/async
Content-Type: application/json
```

**请求体：**

```json
{
  "session_id": "a1b2c3d4-e5f6-7890",
  "message": "请解释什么是快速排序"
}
```

**流式响应：**

```
data: "快速"\n\n
data: "排序"\n\n
data: "（Quick Sort）是一种高效的排序算法..."\n\n
data: [DONE]
```

> 💡 前端可通过 `EventSource` 或 `fetch` + `ReadableStream` 接收流式数据，实现逐字显示效果。

---

## 🧪 测试

```bash
# 编译测试
cd test
mkdir build && cd build
cmake ..
make -j$(nproc)

# 运行 LLM 接口测试
./testLLM

# 运行 SQLite 数据库测试
cd ../testSQLite
g++ -o testSqlite3 testSqlite3.cpp -lsqlite3
./testSqlite3
```

## ❓ 常见问题

<details>
<summary><b>编译时找不到 ai_chat_sdk 头文件</b></summary>

请先编译并安装 SDK：
```bash
cd sdk/build && cmake .. && make && sudo make install
```
确认 `/usr/local/include/ai_chat_sdk/` 目录存在头文件，`/usr/local/lib/libai_chat_sdk.a` 存在。
</details>

<details>
<summary><b>启动报错：至少需要提供一个有效的 API 密钥</b></summary>

需要设置至少一个模型的环境变量：
```bash
export deepseek_apikey="sk-xxx"
```
或者使用 Ollama 本地模型（不需要 API Key）：
```bash
./build/AIChatServer --ollama_model_name=deepseek-r1:1.5b --ollama_endpoint=http://localhost:11434
```
</details>

<details>
<summary><b>如何添加新的 AI 模型？</b></summary>

1. 在 `sdk/include/` 下新建 `XxxProvider.h`，继承 `LLMProvider` 基类
2. 在 `sdk/src/` 下实现 `XxxProvider.cpp`
3. 在 `LLMManager` 中注册新模型
4. 重新编译 SDK 并安装
</details>

<details>
<summary><b>前端页面如何部署？</b></summary>

将前端文件（HTML/CSS/JS）放入 `ChatServer/www/` 目录，服务器会自动托管静态资源。
访问 `http://your-ip:8080/` 即可打开前端页面。
</details>

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

1. Fork 本仓库
2. 创建特性分支（`git checkout -b feature/amazing-feature`）
3. 提交更改（`git commit -m 'Add some amazing feature'`）
4. 推送到分支（`git push origin feature/amazing-feature`）
5. 发起 Pull Request

## 📄 许可证

本项目基于 [MIT License](https://opensource.org/licenses/MIT) 开源。

---

<div align="center">

**如果这个项目对你有帮助，请给个 ⭐ Star 支持一下！**

</div>
