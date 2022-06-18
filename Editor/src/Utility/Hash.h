/************************************************************************************//*!
\file           Hash.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           June 14, 2022
\brief          Contains a utility hashing class that stores the various hashing algorithms
                currently only supports fnv-1a hash.


Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <string>

/********************************************************************************//*!
@brief Contains a utility hashing class that stores the various hashing algorithms
        currently only supports fnv-1a hash.
*//*********************************************************************************/
struct StringHash
{
public:
    using size_type = uint32_t;

    constexpr static size_t const_strlen(char const* s)
    {
        size_t size = 0;
        while (s[size]) { size++; };
        return size;
    }
    /****************************************************************************//*!
    @brief Implementations the FNV-1a hashing algorithm.
        The fnv-1a implementation provides better avalanche characteristics
        against the fnv-1.
        The FNV hash was designed for fast hash tableand checksum use, not cryptography.
        Find out more
        https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function

    @return returns the hashed fnv-1a output.
    *//*****************************************************************************/
    constexpr static size_type  GenerateFNV1aHash(char const* str)
    {
        // Also C++ does not like static constexpr
        constexpr size_type FNV_PRIME = 16777619u;
        constexpr size_type OFFSET_BASIS = 2166136261u;

        const size_t length = const_strlen(str) + 1;
        size_type hash = OFFSET_BASIS;
        for (size_t i = 0; i < length; ++i)
        {
            hash ^= *str++;
            hash *= FNV_PRIME;
        }
        return hash;
    }

    constexpr static size_type GenerateFNV1aHash(std::string_view string)
    {
        return GenerateFNV1aHash(string.data());
    }

public:
    size_type computedHash;

    constexpr StringHash(size_type hash) noexcept : computedHash(hash) {}

    constexpr StringHash(char const* s) noexcept : computedHash(0)
    {
        computedHash = GenerateFNV1aHash(s);
    }

    constexpr StringHash(std::string_view s) noexcept : computedHash(0)
    {
        computedHash = GenerateFNV1aHash(s.data());
    }

    StringHash(const StringHash& other) = default;

    constexpr operator size_type() noexcept { return computedHash; }
    constexpr bool operator==(const StringHash other) noexcept { return computedHash == other.computedHash; }
    //constexpr bool operator<(const StringHash other)noexcept { return computedHash < other.computedHash; }
};