#include "net/udp_client.hpp"

#include <arpa/inet.h>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

UdpClient::UdpClient() : sockfd_(-1) {
    sockfd_ = ::socket(AF_INET, SOCK_DGRAM, 0);
}

UdpClient::~UdpClient() {
    if (sockfd_ >= 0) {
        ::close(sockfd_);
    }
}

bool UdpClient::send_to(const std::string& ip, uint16_t port, const std::string& payload) {
    if (sockfd_ < 0) {
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (::inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) != 1) {
        return false;
    }

    const ssize_t sent = ::sendto(
        sockfd_,
        payload.c_str(),
        payload.size(),
        0,
        reinterpret_cast<const sockaddr*>(&addr),
        sizeof(addr)
    );

    return sent == static_cast<ssize_t>(payload.size());
}