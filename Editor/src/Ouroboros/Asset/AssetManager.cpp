#include "pch.h"

#include "AssetManager.h"

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
    // Get asset from internal info store
    if (internalInfo.contains(snowflake))
    {
        return LoadFile(internalInfo.at(snowflake).originalPath);
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
    const std::filesystem::path DIR = std::filesystem::canonical("./assets/");

    std::chrono::steady_clock::time_point t = std::chrono::steady_clock::now();
    while (isRunning)
    {
        // Check for removal
        //auto it = internalInfo.begin();
        //while (it != internalInfo.end())
        //{
        //    if (!std::filesystem::exists(it->second.originalPath))
        //    {
        //        // Removed
        //        it = internalInfo.erase(it);

        //        std::cout << "unindexed " << it->second.originalPath << "\n";
        //        continue;
        //    }
        //    ++it;
        //}

        // Check for creation or modification
        for (auto& file : std::filesystem::recursive_directory_iterator(DIR))
        {
            const std::filesystem::path FP = std::filesystem::canonical(file.path());
            const std::filesystem::path FP_EXT = FP.extension();
            const auto WRITE_TIME = std::filesystem::last_write_time(file.path());

            // Check converted assets extension
            if (FP_EXT == Asset::EXT_DATA_GENERIC)
            {
                // Read header
                AssetHeader header;
                std::ifstream ifs = std::ifstream(FP);
                BinaryIO::Read(ifs, header);

                if (!internalInfo.contains(header.id))
                {
                    // Newly created
                    internalInfo[header.id] = { .originalPath = FP, .lastWriteTime = WRITE_TIME };

                    std::cout << "indexed " << FP << "\n";
                }
                else if (std::chrono::duration_cast<std::chrono::seconds>(WRITE_TIME - internalInfo[header.id].lastWriteTime).count() > 1)
                {
                    // Updated
                    internalInfo[header.id].lastWriteTime = WRITE_TIME;
                    // TODO: update data
                }
            }
            else
            {
                std::filesystem::path ofp = FP;
                ofp.replace_extension(Asset::EXT_DATA_GENERIC);
                if (!std::filesystem::exists(ofp))
                {
                    // Load file into store
                    LoadFile(FP);

                    std::cout << "loaded " << FP << "\n";
                }
            }

            //if (!assetMap.contains(FP))
            //{
            //    // Created
            //    writeMap[FP.string()] = WRITE_TIME; =
            //}
            //else if (writeMap[FP.string()] != WRITE_TIME)
            //{
            //    // Modified
            //    writeMap[FP.string()] = WRITE_TIME; =
            //}
        }

        // Check time elapsed
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - t).count() < WATCH_INTERVAL)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        t = now;
    }
}
