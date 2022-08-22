#pragma once

#include <iostream>

class BinaryIO final
{
public:
    template<typename T>
    static std::ostream& Write(std::ostream& os, const T& val)
    {
        return os.write(reinterpret_cast<const char*>(&val), sizeof(T));
    }

    template<typename T>
    static std::ostream& Write(std::ostream& os, const T& val, size_t sz)
    {
        return os.write(reinterpret_cast<const char*>(&val), sz);
    }

    template<typename T>
    static std::istream& Read(std::istream& is, T& val)
    {
        return is.read(reinterpret_cast<char*>(&val), sizeof(T));
    }

    template<typename T>
    static std::istream& Read(std::istream& is, T& val, size_t sz)
    {
        return is.read(reinterpret_cast<char*>(&val), sz);
    }

private:
    BinaryIO() = delete;
};