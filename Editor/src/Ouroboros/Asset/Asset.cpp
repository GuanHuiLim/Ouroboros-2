#include "pch.h"

#include "Asset.h"

AssetHeader::AssetHeader(const std::string& ext) : id{ GenerateSnowflake() }
{
    // Copy chraracters
    std::fill_n(extOriginal, EXT_SIZE, 0);
    size_t count = EXT_SIZE;
    if (ext.size() < count)
        count = ext.size();
    std::copy_n(ext.begin(), count, extOriginal);
}

Snowflake AssetHeader::GenerateSnowflake()
{
    // Internal sequence number up to 65535
    static uint16_t sequence = 0;

    // Time at ms precision
    auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now());
    auto time = now.time_since_epoch();

    // 48 bits of time, 16 bits of sequence number
    return (time.count() << 16) | ++sequence;
}

Asset::Asset()
    : header{ new AssetHeader() }
    , meta{ new AssetMeta() }
{
    meta->copies.emplace_back(this);
}

Asset::Asset(Asset& other)
    : header{ other.header }
    , meta{ other.meta }
{
    meta->copies.emplace_back(this);
}

Asset::Asset(Asset&& other)
    : header{ other.header }
    , meta{ other.meta }
{
    meta->copies.remove(&other);
    meta->copies.emplace_back(this);
    other.header = nullptr;
    other.meta = nullptr;
}

Asset& Asset::operator=(Asset& other)
{
    this->Asset::~Asset();
    this->Asset::Asset(other);
    return *this;
}

Asset& Asset::operator=(Asset&& other)
{
    this->Asset::~Asset();
    this->Asset::Asset(std::forward<Asset>(other));
    return *this;
}

Asset::~Asset()
{
    if (header && meta)
    {
        meta->copies.remove(this);
        if (meta->copies.empty())
        {
            // Free
            delete header;
            if (meta->data)
                delete meta->data;
            delete meta;
        }
    }
    header = nullptr;
    meta = nullptr;
}
