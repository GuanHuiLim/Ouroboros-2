#include "pch.h"

#include "AssetManager.h"

#include <iostream>
#include <fstream>

#include "BinaryIO.h"

AssetManager::AssetManager()
{
    fileWatchThread = std::thread(&AssetManager::fileWatch, this);
}

AssetManager::~AssetManager()
{
    isRunning = false;
    fileWatchThread.join();
}

Asset AssetManager::Load(const Snowflake& snowflake)
{
    // Get asset from asset store
    if (assets.contains(snowflake))
    {
        return assets.at(snowflake);
    }
    // Get asset from asset map
    if (assetMap.contains(snowflake))
    {
        return LoadFile(assetMap.at(snowflake));
    }
    throw AssetNotFoundException();
}

std::future<Asset> AssetManager::LoadAsync(const Snowflake& snowflake)
{
    return std::async(std::launch::async, &AssetManager::Load, this, snowflake);
}

Asset AssetManager::LoadFile(const std::filesystem::path& fp)
{
    if (!std::filesystem::exists(fp))
        throw AssetNotFoundException();

    // Check if converted asset
    auto fpAsset = fp;
    if (fpAsset.extension() != Asset::EXT_DATA_GENERIC)
    {
        // Convert
        fpAsset = ConvertPlainAsset(fpAsset);
    }

    // Guarantee asset
    Asset asset;

    // Read header
    std::ifstream ifs = std::ifstream(fpAsset);
    BinaryIO::Read(ifs, *asset.header);

    if (assets.contains(asset.header->id))
    {
        // Acquire asset
        return assets.at(asset.header->id);
    }
    else
    {
        // Store asset
        // TODO: store different data depending on asset type
        asset.meta->data = new std::ifstream(fpAsset);
        assets.insert({ asset.header->id, asset });
        return asset;
    }
}

std::future<Asset> AssetManager::LoadFileAsync(const std::filesystem::path& fp)
{
    return std::async(std::launch::async, &AssetManager::LoadFile, this, fp);
}

std::filesystem::path AssetManager::ConvertPlainAsset(const std::filesystem::path& fp)
{
    // Read file
    std::ifstream ifs = std::ifstream(fp);
    const size_t IF_SIZE = std::filesystem::file_size(fp);

    // Write file
    std::filesystem::path ofp = fp;
    ofp.replace_extension(Asset::EXT_DATA_GENERIC);
    std::ofstream ofs = std::ofstream(ofp);

    // Write header content
    AssetHeader header = AssetHeader(fp.extension().string());
    header.id = AssetHeader::GenerateSnowflake();
    BinaryIO::Write(ofs, header);

    // Transfer contents
    size_t count = IF_SIZE;
    const size_t BUF_SIZE = 4096;
    char buffer[BUF_SIZE];
    while (count > BUF_SIZE)
    {
        ifs.read(buffer, BUF_SIZE);
        ofs.write(buffer, BUF_SIZE);
        count -= BUF_SIZE;
    }
    ifs.read(buffer, count);
    ofs.write(buffer, count);

    return ofp;
}

void AssetManager::fileWatch()
{
    std::chrono::steady_clock::time_point t = std::chrono::steady_clock::now();
    while (isRunning)
    {
        // Check time elapsed
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - t).count() < WATCH_INTERVAL)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // TODO: Look for new files

        t = now;
    }
}
