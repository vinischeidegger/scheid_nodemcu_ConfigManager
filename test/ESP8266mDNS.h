#ifndef MOCK_ESP8266_MDNS_H
#define MOCK_ESP8266_MDNS_H

// No-op mDNS stub for native test compilation.

inline void MDNSbegin(const char*) {}
inline void MDNSaddService(const char*, const char*, const char*) {}

#endif // MOCK_ESP8266_MDNS_H
