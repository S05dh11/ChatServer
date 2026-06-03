#include "../include/SessionManager.h"
#include <iomanip>
#include <sstream>
#include <vector>
#include "../include/util/myLog.h"


namespace ai_chat_sdk {
SessionManager::SessionManager(const std::string& dbName)
    : _dataManager(dbName)
{
    // 获取所有会话
    auto sessions = _dataManager.getAllSessions();
    for(auto& session : sessions){
        _sessions[session->_sessionId] = session;
    }
    // 从数据库已有会话数量初始化计数器，避免ID冲突
    _sessionCounter.store(static_cast<int64_t>(sessions.size()));
}

// 生成会话id  会话id格式：session_时间戳_会话计数
std::string SessionManager::generateSessionId(){
    // 会话计数自增
    _sessionCounter.fetch_add(1);
    std::time_t time = std::time(nullptr);

    // 生成会话id
    std::ostringstream os;
    //  session_1234567890_00000001
    os<<"session_"<<time<<"_"<<std::setw(8)<<std::setfill('0')<<_sessionCounter;
    return os.str();
}

// 生成消息id  msg_1234567890_00000001
std::string SessionManager::generateMessageId(size_t messageCounter){
    messageCounter++;
    std::time_t time = std::time(nullptr);

    std::ostringstream os;
    os<<"msg_"<<time<<"_"<<std::setw(8)<<std::setfill('0')<<messageCounter;
    return os.str();
}

// 创建会话，提供模型名称
std::string SessionManager::createSession(const std::string& modelName){
    std::string sessionId;
    std::shared_ptr<Session> session;

    {
        std::lock_guard<std::mutex> lock(_mutex);
        // 生成会话id
        sessionId = generateSessionId();

        // 创建会话，并设置会话id
        session = std::make_shared<Session>(modelName);
        session->_sessionId = sessionId;
        session->_createdAt = std::time(nullptr);
        session->_updatedAt = session->_createdAt;

        // 将会话入到会话列表
        _sessions[sessionId] = session;
    }

    // 将会话保存到数据库
    _dataManager.insertSession(*session);
    return sessionId;
}

// 通过会话ID获取会话信息
std::shared_ptr<Session> SessionManager::getSession(const std::string& sessionId){
    // 先在内存中查找
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _sessions.find(sessionId);
        if(it != _sessions.end()){
            // 获取当前会话的历史消息
            it->second->_messages = _dataManager.getSessionMessages(sessionId);
            return it->second;
        }
    }

    // 内存中没有找到，从数据库中查找
    auto session = _dataManager.getSession(sessionId);
    if(session){
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _sessions.find(sessionId);
        if(it == _sessions.end()){
            // 内存中没有找到，将会话添加到会话列表
            _sessions[sessionId] = session;
        }

        // 获取当前会话的历史消息
        session->_messages = _dataManager.getSessionMessages(sessionId);
        return session;
    }

    WARN("sessionId = {} not found", sessionId);
    return nullptr;
}

// 通过会话ID获取会话元数据（不加载历史消息，用于列表展示）
std::shared_ptr<Session> SessionManager::getSessionMeta(const std::string& sessionId){
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _sessions.find(sessionId);
        if(it != _sessions.end()){
            return it->second;
        }
    }

    // 内存中没有，从数据库查（getSession内部会加载消息，这里只需要元数据）
    auto session = _dataManager.getSession(sessionId);
    if(session){
        std::lock_guard<std::mutex> lock(_mutex);
        if(_sessions.find(sessionId) == _sessions.end()){
            _sessions[sessionId] = session;
        }
        return session;
    }
    return nullptr;
}   

// 往某个会话中添加消息
bool SessionManager::addMessage(const std::string& sessionId, const Message& message){
    Message msg;

    {
        std::lock_guard<std::mutex> lock(_mutex);

        // 获取到sessionId对应的会话
        auto it = _sessions.find(sessionId);
        if(it == _sessions.end()){
            return false;
        }

        // 创建消息
        msg = Message(message._role, message._content);
        msg._messageId = generateMessageId(it->second->_messages.size());
        msg._timestamp = std::time(nullptr);  // 设置消息时间戳
        INFO("message Info: content {}  timestamap {}", msg._content, msg._timestamp);

        // 将消息添加到会话中
        it->second->_messages.push_back(msg);
        it->second->_updatedAt = std::time(nullptr);
        INFO("add message success, sessionId = {}, message.content = {}", sessionId, msg._content);
    }

    // 将会话保存到数据库
    _dataManager.insertMessage(sessionId, msg);
    return true;
}

// 获取某个会话的所有历史消息
std::vector<Message> SessionManager::getHistroyMessages(const std::string& sessionId)const{
    // 先从内存中获取会话消息，如果内存中获取不到，再到数据库中获取
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _sessions.find(sessionId);
        if(it != _sessions.end()){
            return it->second->_messages;
        }
    }

    // 从数据库中获取消息列表
    return _dataManager.getSessionMessages(sessionId);
}

// 更新会话时间戳
void SessionManager::updateSessionTimestamp(const std::string& sessionId){
    std::time_t newTime = std::time(nullptr);
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _sessions.find(sessionId);
        if(it != _sessions.end()){
            it->second->_updatedAt = newTime;
        }
    }

    // 更新数据库中的会话时间戳
    _dataManager.updateSessionTimestamp(sessionId, newTime);
}

// 获取所有会话列表
std::vector<std::string> SessionManager::getSessionLists()const{
    auto sessions = _dataManager.getAllSessions();

    std::lock_guard<std::mutex> lock(_mutex);
    // 构建一个临时对话列表，将其内容的会话按照会话更新的时间戳降序排列
    std::vector<std::pair<std::time_t, std::shared_ptr<Session>>> temp;
    temp.reserve(_sessions.size());

    // 将会话添加到临时列表中
    for(const auto& pair : _sessions){
        temp.emplace_back(pair.second->_updatedAt, pair.second);
    }

    // 将数据库中的会话添加到临时列表中
    for(const auto& session : sessions){
        if(_sessions.find(session->_sessionId) == _sessions.end()){
            temp.emplace_back(session->_updatedAt, session);
        }
    }

    // 按照会话更新时间戳降序排序
    std::sort(temp.begin(), temp.end(), [](const auto& a, const auto& b){
        return a.first > b.first;
    });

    std::vector<std::string> sessionIds;
    sessionIds.reserve(_sessions.size());
    // 从临时列表中提取会话id
    for(const auto& pair : temp){
        sessionIds.push_back(pair.second->_sessionId);
    }
    return sessionIds;
}

// 删除某个会话
bool SessionManager::deleteSession(const std::string& sessionId){
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _sessions.find(sessionId);
        if(it != _sessions.end()){
            // 从内存中删除会话
            _sessions.erase(it);
        }
    }

    // 从数据库中删除会话（无论内存中是否存在都要删）
    return _dataManager.deleteSession(sessionId);
}

// 清空所有会话
void SessionManager::clearAllSessions(){
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _sessions.clear();
    }

    // 清空数据库中的所有会话
    _dataManager.clearAllSessions();
}

// 获取会话总数
size_t SessionManager::getSessionCount()const{
    std::lock_guard<std::mutex> lock(_mutex);
    return _sessions.size();
}

// 获取指定会话的消息数量
size_t SessionManager::getMessageCount(const std::string& sessionId)const{
    return _dataManager.getSessionMessageCount(sessionId);
}

// 获取指定会话的第一条用户消息
std::string SessionManager::getFirstUserMessage(const std::string& sessionId)const{
    return _dataManager.getFirstUserMessage(sessionId);
}


} // end ai_chat_sdk
