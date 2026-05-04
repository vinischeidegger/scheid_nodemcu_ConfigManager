#ifndef MOCK_ARDUINO_H
#define MOCK_ARDUINO_H

// Mock Arduino types and functions for native testing
#include <string>
#include <vector>
#include <iostream>

// Mock String class
class String {
public:
    String() : _str() {}
    String(const char* s) : _str(s) {}
    String(const std::string& s) : _str(s) {}

    const char* c_str() const { return _str.c_str(); }
    size_t length() const { return _str.length(); }
    bool operator==(const String& other) const { return _str == other._str; }
    bool operator==(const char* other) const { return _str == other; }
    String& operator=(const String& other) { _str = other._str; return *this; }
    String& operator=(const char* other) { _str = other; return *this; }

private:
    std::string _str;
};


// Mock Serial
class SerialClass {
public:
    void println(const char* msg) { std::cout << msg << std::endl; }
    void println(const String& msg) { std::cout << msg.c_str() << std::endl; }
    void print(const char* msg) { std::cout << msg; }
    void print(const String& msg) { std::cout << msg.c_str(); }
};

inline SerialClass Serial;

// Mock delay
inline void delay(int ms) { /* mock delay */ }

// Mock PROGMEM
#define PROGMEM

#endif // MOCK_ARDUINO_H