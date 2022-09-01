/************************************************************************************//*!
\file           BinaryIO.h
\project        Ouroboros
\author         Tay Yan Chong Clarence, t.yanchongclarence, 620008720 | code contribution (100%)
\par            email: t.yanchongclarence\@digipen.edu
\date           Aug 30, 2022
\brief          Contains the definition for the BinaryIO helper class.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/

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