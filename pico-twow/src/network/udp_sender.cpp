#include "network/udp_sender.hpp"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

UdpSender::UdpSender(const std::string& destination_ip, uint16_t destination_port)
    : destination_ip_(destination_ip), destination_port_(destination_port) {}

UdpSender::~UdpSender() {
    if (sockfd_ >= 0) {
        ::close(sockfd_);
        sockfd_ = -1;
    }
}

bool UdpSender::initialize() {
    sockfd_ = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd_ < 0) {
        std::perror("socket");
        return false;
    }
    return true;
}

bool UdpSender::send(const std::string& payload) {
    sockaddr_in dest{};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(destination_port_);

    if (::inet_pton(AF_INET, destination_ip_.c_str(), &dest.sin_addr) != 1) {
        std::cerr << "Invalid destination IP: " << destination_ip_ << "\n";
        return false;
    }

    ssize_t sent = ::sendto(
        sockfd_,
        payload.data(),
        payload.size(),
        0,
        reinterpret_cast<sockaddr*>(&dest),
        sizeof(dest)
    );

    return sent == static_cast<ssize_t>(payload.size());
}
