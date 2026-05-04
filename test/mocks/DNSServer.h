#ifndef MOCK_DNS_SERVER_H
#define MOCK_DNS_SERVER_H

class DNSServer {
public:
    DNSServer() = default;
    void start(unsigned char, const char*, const char*) {}
};

#endif // MOCK_DNS_SERVER_H
