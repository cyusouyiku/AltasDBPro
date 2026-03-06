#include "ServerManager.h"
#include "Transaction.h"
#include "TransactionManager.h"
#include "parser/SQLParser.h"
#include "optimizer/QueryOptimizer.h"
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

ServerManager::ServerManager()
    : server_socket_(-1),
      server_port_(0),
      is_running_(false),
      should_stop_(false),
      max_clients_(128),
      thread_pool_size_(4),
      txn_manager_(nullptr),
      parser_(nullptr),
      optimizer_(nullptr),
      total_requests_(0),
      active_connections_(0) {}

ServerManager::~ServerManager() {
    StopServer();
    CleanupResources();
}

void ServerManager::CleanupResources() {
    if (server_socket_ >= 0) {
        close(server_socket_);
        server_socket_ = -1;
    }
    for (auto& p : clients_) {
        if (p.second) {
            close(p.second->fd);
            delete p.second;
        }
    }
    clients_.clear();
}

bool ServerManager::InitializeSocket(int port) {
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ < 0) {
        return false;
    }
    int opt = 1;
    setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(static_cast<uint16_t>(port));
    if (bind(server_socket_, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        close(server_socket_);
        server_socket_ = -1;
        return false;
    }
    if (listen(server_socket_, max_clients_) < 0) {
        close(server_socket_);
        server_socket_ = -1;
        return false;
    }
    server_port_ = port;
    return true;
}

bool ServerManager::StartServer(int port) {
    if (!InitializeSocket(port)) {
        return false;
    }
    is_running_ = true;
    should_stop_ = false;
    InitializeThreadPool();
    return true;
}

void ServerManager::StopServer() {
    should_stop_ = true;
    is_running_ = false;
    queue_cv_.notify_all();
    for (auto& t : worker_threads_) {
        if (t.joinable()) {
            t.join();
        }
    }
    worker_threads_.clear();
    CleanupResources();
}

bool ServerManager::IsRunning() const {
    return is_running_;
}

void ServerManager::InitializeThreadPool() {
    worker_threads_.reserve(static_cast<size_t>(thread_pool_size_));
    for (int i = 0; i < thread_pool_size_; ++i) {
        worker_threads_.emplace_back(&ServerManager::WorkerThreadLoop, this);
    }
}

bool ServerManager::SetNonBlocking(int) {
    return true;
}

void ServerManager::AcceptConnections() {}

bool ServerManager::AcceptClient() {
    return false;
}

void ServerManager::DisconnectClient(int client_fd) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    auto it = clients_.find(client_fd);
    if (it != clients_.end()) {
        if (it->second) {
            close(it->second->fd);
            delete it->second;
        }
        clients_.erase(it);
    }
}

bool ServerManager::HandleClientRequest(const std::string&) {
    return false;
}

void ServerManager::EnqueueRequest(ClientRequest* req) {
    if (!req) return;
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        request_queue_.push(req);
    }
    queue_cv_.notify_one();
}

void ServerManager::WorkerThreadLoop() {
    while (!should_stop_) {
        ClientRequest* req = nullptr;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [this] { return should_stop_ || !request_queue_.empty(); });
            if (should_stop_) break;
            if (request_queue_.empty()) continue;
            req = request_queue_.front();
            request_queue_.pop();
        }
        if (req) {
            HandleClientRequest(req->request);
            SendResponse(req->client_fd, "");
            delete req;
        }
    }
}

std::string ServerManager::ProcessSQLRequest(const std::string& sql, int) {
    if (optimizer_) {
        std::string optimized = optimizer_->OptimizeQuery(sql);
        return optimized;
    }
    return sql;
}

std::string ServerManager::ExecuteQuery(const std::string&, Transaction*) {
    return "";
}

std::string ServerManager::HandleTransaction(const std::string&, int) {
    return "";
}

Session* ServerManager::GetOrCreateSession(int) {
    return nullptr;
}

void ServerManager::CloseSession(int client_fd) {
    DisconnectClient(client_fd);
}

std::string ServerManager::SerializeResult(const QueryResult& result) {
    return result.data;
}

void ServerManager::SendResponse(int, const std::string&) {}

void ServerManager::LogRequest(const std::string&, int) {}
