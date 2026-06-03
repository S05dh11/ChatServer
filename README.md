# 🤖 AIChatServer

基于 C++17 构建的 AI 智能聊天服务器，支持多模型接入、流式对话（SSE）、会话管理，提供 RESTful API 接口。

## ✨ 功能特性

- **多模型支持** — DeepSeek、豆包（Doubao）、通义千问（Qwen）、Ollama 本地模型
- **流式响应** — 基于 SSE（Server-Sent Events）的实时流式输出
- **全量响应** — 支持一次性返回完整消息
- **会话管理** — 创建、删除、查询会话及历史消息
- **配置灵活** — 支持命令行参数、配置文件、环境变量三种配置方式
- **静态资源托管** — 内置前端页面服务，开箱即用

## 🛠️ 技术栈

| 组件 | 技术 |
|------|------|
| HTTP 框架 | [cpp-httplib](https://github.com/yhirose/cpp-httplib) |
| AI SDK | ai_chat_sdk（自研） |
| JSON 解析 | jsoncpp |
| 命令行参数 | gflags |
| 日志 | spdlog |
| 格式化 | fmt |
| 数据库 | SQLite3 |
| SSL | OpenSSL |

## 📁 项目结构

```
ChatServer/
├── ChatServer/                # 聊天服务器主程序
│   ├── CMakeLists.txt         # CMake 构建配置
│   ├── ChatServer.h           # 服务器类声明
│   ├── ChatServer.cpp         # 服务器类实现（路由、请求处理）
│   ├── main.cpp               # 程序入口（参数解析、配置校验）
│   ├── required/              # 需求文档
│   │   ├── main需求.txt
│   │   └── Chat前端页面需求说明.txt
│   └── www/                   # 前端静态资源目录（需自行放置）
│
├── sdk/                       # AI ChatSDK — 多模型接入 SDK
│   ├── CMakeLists.txt         # SDK 构建配置（生成静态库）
│   ├── include/               # SDK 头文件
│   │   ├── ChatSDK.h          # SDK 主接口
│   │   ├── LLMProvider.h      # LLM 提供者基类
│   │   ├── LLMManager.h       # LLM 管理器
│   │   ├── SessionManager.h   # 会话管理器
│   │   ├── DataManager.h      # 数据持久化管理
│   │   ├── DeepSeekProvider.h # DeepSeek 模型接入
│   │   ├── DoubaoProvider.h   # 豆包模型接入
│   │   ├── QwenProvider.h     # 通义千问模型接入
│   │   ├── OllamaLLMProvider.h # Ollama 本地模型接入
│   │   ├── common.h           # 公共类型定义
│   │   └── util/myLog.h       # 日志工具
│   └── src/                   # SDK 源文件
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
└── test/                      # 测试代码
    ├── CMakeLists.txt
    ├── testLLM.cpp            # LLM 接口测试
    └── testSQLite/
        └── testSqlite3.cpp    # SQLite 数据库测试
```

## 📦 编译依赖

- CMake ≥ 3.10
- G++（支持 C++17）
- OpenSSL 开发库
- SQLite3 开发库
- jsoncpp、spdlog、fmt、gflags

Ubuntu / Debian 安装依赖：

```bash
sudo apt update
sudo apt install build-essential cmake libssl-dev libsqlite3-dev libjsoncpp-dev libspdlog-dev libfmt-dev libgflags-dev
```

## 🔨 编译构建

### 1. 编译安装 AI ChatSDK（静态库）

```bash
cd sdk
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install    # 安装静态库到 /usr/local/lib，头文件到 /usr/local/include/ai_chat_sdk
```

### 2. 编译 ChatServer

```bash
cd ChatServer
mkdir build && cd build
cmake ..
make -j$(nproc)
```

编译成功后生成可执行文件 `build/AIChatServer`。

### 3. 编译测试（可选）

```bash
cd test
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## ⚙️ 配置说明

### 环境变量（API Key）

```bash
export deepseek_apikey="your_deepseek_api_key"
export doubao_apikey="your_doubao_api_key"
export qwen_apikey="your_qwen_api_key"
```

> 至少配置一个 API Key，否则启动失败。

### 命令行参数

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `--host` | `0.0.0.0` | 服务器绑定地址 |
| `--port` | `8080` | 服务器绑定端口 |
| `--log_level` | `INFO` | 日志级别 |
| `--temperature` | `0.7` | 温度值（0.0 ~ 2.0） |
| `--max_tokens` | `2048` | 最大 token 数 |
| `--config_file` | `./ChatServer.conf` | 配置文件路径 |
| `--ollama_model_name` | — | Ollama 模型名称 |
| `--ollama_model_desc` | — | Ollama 模型描述 |
| `--ollama_endpoint` | — | Ollama API 地址 |

### 配置文件

将参数写入 `ChatServer.conf`，程序启动时自动加载：

```
--host=0.0.0.0
--port=8080
--temperature=0.7
--max_tokens=2048
```

## 🚀 启动运行

```bash
# 基本启动
./build/AIChatServer

# 指定端口
./build/AIChatServer --port=9000

# 指定配置文件
./build/AIChatServer --config_file=my_config.conf

# 使用 Ollama 本地模型
./build/AIChatServer --ollama_model_name=deepseek-r1:1.5b --ollama_model_desc="本地模型" --ollama_endpoint=http://localhost:11434

# 查看帮助
./build/AIChatServer --help

# 查看版本
./build/AIChatServer --version
```

## 📡 API 接口

### 创建会话

```
POST /api/session
Content-Type: application/json

{"model": "deepseek-chat"}
```

### 获取会话列表

```
GET /api/sessions
```

### 获取可用模型

```
GET /api/models
```

### 删除会话

```
DELETE /api/session/{session_id}
```

### 获取历史消息

```
GET /api/session/{session_id}/history
```

### 发送消息（全量返回）

```
POST /api/message
Content-Type: application/json

{"session_id": "xxx", "message": "你好"}
```

### 发送消息（流式返回 SSE）

```
POST /api/message/async
Content-Type: application/json

{"session_id": "xxx", "message": "你好"}
```

流式响应格式：

```
data: "文本内容"\n\n
data: "更多内容"\n\n
data: [DONE]
```

## 📄 许可证

MIT License
