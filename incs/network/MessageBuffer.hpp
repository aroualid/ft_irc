#pragma once

#include <vector>

class MessageBuffer {
private:
    std::string _readBuffer;
    std::string _writeBuffer;

public:
    void appendRead(const std::string& data);
    bool hasCompleteMessage() const;
    std::string getNextMessage();

    void appendWrite(const std::string& data);
    const std::string& getWriteBuffer() const;
    void consumeWriteBuffer(size_t bytes);
};