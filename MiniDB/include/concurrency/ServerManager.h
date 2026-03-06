#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include "Transaction.h"
#include "TransactionManager.h"
#include "parser/SQLParser.h"
#include "optimizer/QueryOptimizer.h"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <cstdint>

struct ClientConnection {
    int fd;
};

struct ClientRequest {
    std::string request;
    int client_fd;
};

struct QueryResult {
    std::string data;
};

struct Session {
    int client_fd;
    Transaction* txn;
};

class ServerManager {
private:
    int server_socket_;
    int server_port_;
    bool is_running_;
    std::atomic<bool> should_stop_;
    std::unordered_map<int, ClientConnection*> clients_;
    std::mutex clients_mutex_;
    int max_clients_;
    std::vector<std::thread> worker_threads_;
    int thread_pool_size_;
    std::queue<ClientRequest*> request_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    TransactionManager* txn_manager_;
    SQLParser* parser_;
    QueryOptimizer* optimizer_;
    std::atomic<uint64_t> total_requests_;
    std::atomic<uint64_t> active_connections_;

public:
    ServerManager();
    ~ServerManager();
    bool InitializeSocket(int port);
    bool StartServer(int port);
    void StopServer();
    bool IsRunning() const;
    void AcceptConnections();
    bool AcceptClient();
    void DisconnectClient(int client_fd);
    bool HandleClientRequest(const std::string& request);
    void EnqueueRequest(ClientRequest* req);
    void WorkerThreadLoop();
    std::string ProcessSQLRequest(const std::string& sql, int client_fd);
    std::string ExecuteQuery(const std::string& sql, Transaction* txn);
    std::string HandleTransaction(const std::string& cmd, int client_fd);
    Session* GetOrCreateSession(int client_fd);
    void CloseSession(int client_fd);
    std::string SerializeResult(const QueryResult& result);
    void SendResponse(int client_fd, const std::string& response);
    void LogRequest(const std::string& sql, int client_fd);

private:
    void InitializeThreadPool();
    void CleanupResources();
    bool SetNonBlocking(int fd);
};

#endif
