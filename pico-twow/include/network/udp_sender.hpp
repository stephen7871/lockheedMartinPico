#pragma once

#include <cstdint>
#include <string>

class UdpSender {
public:
    UdpSender(const std::string& destination_ip, uint16_t destination_port);
    ~UdpSender();

    bool initialize();
    bool send(const std::string& payload);

private:
    std::string destination_ip_;
    uint16_t destination_port_;
    int sockfd_ = -1;
};
