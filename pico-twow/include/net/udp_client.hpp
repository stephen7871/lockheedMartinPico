#pragma once

#include <cstdint>
#include <string>

class UdpClient {
public:
    UdpClient();
    ~UdpClient();

    bool send_to(const std::string& ip, uint16_t port, const std::string& payload);

private:
    int sockfd_;
};