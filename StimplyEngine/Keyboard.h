#pragma once

#include <bitset>

class Keyboard
{
    friend class Window;
public:
    Keyboard(const Keyboard&) = default;
    Keyboard(Keyboard&&) = delete;
    
    bool IsKeyPressed(unsigned char keyCode)
    {
        return _keyStatus[keyCode];
    }

private:
    Keyboard()
    {
        _keyStatus.reset();
    }
    
    void SetKeyPressed(unsigned char keyCode)
    {
        _keyStatus[keyCode] = true;
    }
    void SetKeyReleased(unsigned char keyCode)
    {
        _keyStatus[keyCode] = false;
    }
private:
    std::bitset<256> _keyStatus;
};

