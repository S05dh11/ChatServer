#include "../include/DoubaoProvider.h"
#include "../include/util/myLog.h"
#include <cstdint>
#include <jsoncpp/json/json.h>
#include <httplib.h>
#include <jsoncpp/json/reader.h>
#include <regex>

namespace ai_chat_sdk{

// 去除豆包思维模式返回的<think ... </think标签内容
static std::string stripThinkingContent(const std::string& content){
    std::string result = content;
    std::regex thinkRegex(R"(<think[\s\S]*?</think\s*>)");
    result = std::regex_replace(result, thinkRegex, "");
    size_t start = result.find_first_not_of(" \t\n\r");
    if(start != std::string::npos){
        result = result.substr(start);
    }
    return result;
}

// API调用用的模型端点ID
static const char* DOUBAO_API_MODEL = "ep-20260524113110-tf7mf";

// 初始化模型
bool DoubaoProvider::initModel(const std::map<std::string, std::string>& modelConfig){
    // 初始化API Key
    auto it = modelConfig.find("api_key");
    if(it == modelConfig.end()){
        ERR("DoubaoProvider initModel api_key not found");
        return false;
    }else{
        _apiKey = it->second;
    }

    // 初始化Base URL
    it = modelConfig.find("endpoint");
    if(it == modelConfig.end()){
        _endpoint = "https://ark.cn-beijing.volces.com";
    }else{
        _endpoint = it->second;
    }

    _isAvailable = true;
    INFO("DoubaoProvider initModel success, endpoint: {}", _endpoint);
    return true;
}

// 检测模型是否可用
bool DoubaoProvider::isAvailable() const{
    return _isAvailable;
}

// 获取模型名称（对外显示用）
std::string DoubaoProvider::getModelName() const{
    return "doubao-pro";
}

// 获取模型描述
std::string DoubaoProvider::getModelDesc() const{
    return "字节跳动推出的豆包大模型，中文理解能力强，适合多轮对话与内容创作";
}

// 发送消息 - 全量返回
std::string DoubaoProvider::sendMessage(const std::vector<Message>& messages, const std::map<std::string, std::string>& requestParam)
{
    // 1. 检测模型是否可用
    if(!isAvailable()){
        ERR("DoubaoProvider sendMessage model not available");
        return "";
    }

    // 2. 构造请求参数
    double temperature = 0.7;
    int maxTokens = 2048;
    if(requestParam.find("temperature") != requestParam.end()){
        temperature = std::stod(requestParam.at("temperature"));
    }
    if(requestParam.find("max_tokens") != requestParam.end()){
        maxTokens = std::stoi(requestParam.at("max_tokens"));
    }

    // 构造历史消息
    Json::Value messageArray(Json::arrayValue);
    for(const auto& message : messages){
        Json::Value messageObject;
        messageObject["role"] = message._role;
        messageObject["content"] = message._content;
        messageArray.append(messageObject);
    }

    // 3. 构造请求体
    Json::Value requestBody;
    requestBody["model"] = DOUBAO_API_MODEL;
    requestBody["messages"] = messageArray;
    requestBody["temperature"] = temperature;
    requestBody["max_tokens"] = maxTokens;

    // 4. 序列化
    Json::StreamWriterBuilder writerBuilder;
    writerBuilder["indentation"] = "";
    std::string requestBodyStr = Json::writeString(writerBuilder, requestBody);
    INFO("DoubaoProvider sendMessage requestBody: {}", requestBodyStr);

    // 5. 创建HTTP客户端
    httplib::Client client(_endpoint.c_str());
    client.set_connection_timeout(30, 0);
    client.set_read_timeout(60, 0);

    // 设置请求头
    httplib::Headers headers = {
        {"Authorization", "Bearer " + _apiKey},
        {"Content-Type", "application/json"}
    };

    // 6. 发送POST请求
    auto response = client.Post("/api/v3/chat/completions", headers, requestBodyStr, "application/json");
    if(!response){
        ERR("DoubaoProvider sendMessage POST request failed");
        return "";
    }
    INFO("DoubaoProvider sendMessage POST request success, status : {}", response->status);

    // 检测响应是否成功
    if(response->status != 200){
        return "";
    }

    // 7. 解析响应体
    Json::Value responseBody;
    Json::CharReaderBuilder readerBuilder;
    std::string parseError;
    std::istringstream responseStream(response->body);
    if(Json::parseFromStream(readerBuilder, responseStream, &responseBody, &parseError)){
        if(responseBody.isMember("choices") && responseBody["choices"].isArray() && !responseBody["choices"].empty()){
            auto choice = responseBody["choices"][0];
            if(choice.isMember("message") && choice["message"].isMember("content")){
                std::string replyContent = choice["message"]["content"].asString();
                replyContent = stripThinkingContent(replyContent);
                INFO("DoubaoProvider response text: {}", replyContent);
                return replyContent;
            }
        }
    }

    // 8. json解析失败
    ERR("DoubaoProvider sendMessage POST response body parse failed");
    return "";
}

// 发送消息 - 增量返回 - 流式响应
std::string DoubaoProvider::sendMessageStream(const std::vector<Message>& messages,
                                             const std::map<std::string, std::string>& requestParam,
                                             std::function<void(const std::string&, bool)> callback)
{
    // 1. 检测模型是否可用
    if(!isAvailable()){
        ERR("DoubaoProvider sendMessageStream model not available");
        return "";
    }

    // 2. 构造请求参数
    double temperature = 0.7;
    int maxTokens = 2048;
    if(requestParam.find("temperature") != requestParam.end()){
        temperature = std::stod(requestParam.at("temperature"));
    }
    if(requestParam.find("max_tokens") != requestParam.end()){
        maxTokens = std::stoi(requestParam.at("max_tokens"));
    }

    // 构造历史消息
    Json::Value messageArray(Json::arrayValue);
    for(const auto& message : messages){
        Json::Value messageObject;
        messageObject["role"] = message._role;
        messageObject["content"] = message._content;
        messageArray.append(messageObject);
    }

    // 3. 构造请求体
    Json::Value requestBody;
    requestBody["model"] = DOUBAO_API_MODEL;
    requestBody["messages"] = messageArray;
    requestBody["temperature"] = temperature;
    requestBody["max_tokens"] = maxTokens;
    requestBody["stream"] = true;

    // 4. 序列化
    Json::StreamWriterBuilder writerBuilder;
    writerBuilder["indentation"] = "";
    std::string requestBodyStr = Json::writeString(writerBuilder, requestBody);
    INFO("DoubaoProvider sendMessageStream requestBody: {}", requestBodyStr);

    // 5. 创建HTTP客户端
    httplib::Client client(_endpoint.c_str());
    client.set_connection_timeout(30, 0);
    client.set_read_timeout(300, 0);

    // 设置请求头
    httplib::Headers headers = {
        {"Authorization", "Bearer " + _apiKey},
        {"Content-Type", "application/json"},
        {"Accept", "text/event-stream"}
    };

    // 流式处理变量
    std::string buffer;
    bool gotError = false;
    std::string errorMsg;
    int statusCode = 0;
    bool streamFinish = false;
    std::string fullResponse;

    // 创建请求对象
    httplib::Request req;
    req.method = "POST";
    req.path = "/api/v3/chat/completions";
    req.headers = headers;
    req.body = requestBodyStr;

    // 设置响应处理器
    req.response_handler = [&](const httplib::Response& res) {
        if(res.status != 200){
            gotError = true;
            errorMsg = "HTTP status code: " + std::to_string(res.status);
            return false;
        }
        return true;
    };

    // 设置数据接收处理器
    req.content_receiver = [&](const char* data, size_t len, size_t offset, size_t totalLength){
        if(gotError){
            return false;
        }

        buffer.append(data, len);

        size_t pos = 0;
        while((pos = buffer.find("\n\n")) != std::string::npos){
            std::string chunk = buffer.substr(0, pos);
            buffer.erase(0, pos + 2);

            if(chunk.empty() || chunk[0] == ':'){
                continue;
            }

            if(chunk.compare(0, 6, "data: ") == 0){
                std::string modelData = chunk.substr(6);

                if(modelData == "[DONE]"){
                    callback("", true);
                    streamFinish = true;
                    return true;
                }

                // 反序列化
                Json::Value modelDataJson;
                Json::CharReaderBuilder reader;
                std::string errors;
                std::istringstream modelDataStream(modelData);
                if(Json::parseFromStream(reader, modelDataStream, &modelDataJson, &errors)){
                    if(modelDataJson.isMember("choices") &&
                      modelDataJson["choices"].isArray() &&
                      !modelDataJson["choices"].empty() &&
                      modelDataJson["choices"][0].isMember("delta") &&
                      modelDataJson["choices"][0]["delta"].isMember("content")){
                        std::string content = modelDataJson["choices"][0]["delta"]["content"].asString();
                        fullResponse += content;
                        callback(content, false);
                    }
                }else{
                    WARN("DoubaoProvider sendMessageStream parse modelDataJson error: {}", errors);
                }
            }
        }
        return true;
    };

    // 发送请求
    auto result = client.send(req);
    if(!result){
        ERR("Network error {}", to_string(result.error()));
        return "";
    }

    // 确保流式操作正确结束
    if(!streamFinish){
        WARN("stream ended without [DONE] marker");
        callback("", true);
    }

    return stripThinkingContent(fullResponse);
}


} // end ai_chat_sdk
