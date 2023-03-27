#include "pch.h"
#include "AssetBundle.h"
namespace RRES
{
    //namespace FILE_TYPES
    //{
    //    const unsigned char* _NULL = []() { return reinterpret_cast<const unsigned char*>("NULL"); }();
    //    const unsigned char* _RAWD = []() { return reinterpret_cast<const unsigned char*>("RAWD"); }();
    //    const unsigned char* _TEXT = []() { return reinterpret_cast<const unsigned char*>("TEXT"); }();
    //    const unsigned char* _IMGE = []() { return reinterpret_cast<const unsigned char*>("IMGE"); }();
    //    const unsigned char* _WAVE = []() { return reinterpret_cast<const unsigned char*>("WAVE"); }();
    //    const unsigned char* _VRTX = []() { return reinterpret_cast<const unsigned char*>("VRTX"); }();
    //    const unsigned char* _FNTG = []() { return reinterpret_cast<const unsigned char*>("FNTG"); }();
    //    const unsigned char* _LINK = []() { return reinterpret_cast<const unsigned char*>("LINK"); }();
    //    const unsigned char* _CDIR = []() { return reinterpret_cast<const unsigned char*>("CDIR"); }();
    //}; //namespace FILE_TYPES

Asset::Asset(const char* filename, const unsigned char* type, rresCompressionType compType, rresEncryptionType cipherType, unsigned int flags)
{
    unsigned int rawSize = 0;
    /*----------read data from file into a buffer(m_data) and calculate raw size------------*/
    std::ifstream input_file(filename, std::ios::binary);
    if (input_file.is_open() == false)
    {
        throw std::runtime_error("Failed to open file");
    }
    input_file.seekg(0, std::ios::end);
    rawSize = static_cast<unsigned int>(input_file.tellg());
    input_file.seekg(0, std::ios::beg);
    std::unique_ptr<unsigned char[]> buffer(new unsigned char[rawSize]);
    m_data = std::move(buffer);
    input_file.read(reinterpret_cast<char*>(m_data.get()), rawSize);
    input_file.close();
    /*-------------------------------------------------------------------------------------*/

    /*----------Define chunk info------------*/
    m_info.id = CalculateChunkID(filename);
    m_info.compType = compType;
    m_info.cipherType = cipherType;
    m_info.flags = flags;
    m_info.baseSize = CHUNK_DATA_PROPS_SIZE + rawSize;
    m_info.packedSize = m_info.baseSize;
    m_info.nextOffset = 0;
    m_info.reserved = 0;
    /*---------------------------------------*/

    /*----------Define chunk data------------*/
    m_chunkData.props[0] = rawSize;
	m_chunkData.raw = m_data.get();
    /*---------------------------------------*/

}
    
inline AssetBundle::AssetBundle(const char* filename)
    :m_filename{ filename }
{
    
}

inline AssetBundle::~AssetBundle()
{
    // m_file will be closed automatically when it goes out of scope
}

inline void AssetBundle::AddAsset(const char* filepath, const unsigned char* type, rresCompressionType compType, rresEncryptionType cipherType, unsigned int flags)
{
	m_assets.emplace_back(filepath, type, compType, cipherType, flags);
    m_header.chunkCount = static_cast<unsigned short>(m_assets.size());

    /*
    std::ifstream input_file(filename, std::ios::binary);
    if (!input_file.is_open())
    {
        throw std::runtime_error("Failed to open file");
    }
    unsigned int rawSize = 0;
    input_file.seekg(0, std::ios::end);
    rawSize = static_cast<unsigned int>(input_file.tellg());
    input_file.seekg(0, std::ios::beg);
    std::unique_ptr<unsigned char[]> buffer(new unsigned char[rawSize]);
    input_file.read(reinterpret_cast<char*>(buffer.get()), rawSize);
    input_file.close();

    // Define chunk info
    rresResourceChunkInfo chunkInfo = { 0 };
    memcpy(chunkInfo.type, type, sizeof(chunkInfo.type));
    std::string filename_str(filename);
    chunkInfo.id = rresComputeCRC32(reinterpret_cast<unsigned char*>(filename_str.data()), strlen(filename));
    chunkInfo.compType = compType;
    chunkInfo.cipherType = cipherType;
    chunkInfo.flags = flags;
    chunkInfo.baseSize = 5 * sizeof(unsigned int) + rawSize;
    chunkInfo.packedSize = chunkInfo.baseSize;
    chunkInfo.nextOffset = 0;
    chunkInfo.reserved = 0;

    // Define chunk data
    rresResourceChunkData chunkData = { 0 };
    chunkData.propCount = 1;
    chunkData.props = new unsigned int[chunkData.propCount];
    chunkData.props[0] = rawSize;

    m_file.write(reinterpret_cast<const char*>(&chunkInfo), sizeof(rresResourceChunkInfo));
    m_file.write(reinterpret_cast<const char*>(chunkData.props), sizeof(unsigned int) * chunkData.propCount);
    m_file.write(reinterpret_cast<const char*>(buffer.get()), rawSize);

    // Free resources
    delete[] chunkData.props;

    // Increment chunk count in header
    ++m_header.chunkCount;
    */
}

inline void AssetBundle::WriteToFile()
{
    WriteToFile(m_filename.c_str());
}

void WriteAssetToFile(std::ofstream& m_file, Asset const& asset)
{
    //write chunk data
    m_file.write(reinterpret_cast<const char*>(&asset.m_info), sizeof(asset.m_info));

    /*---------------------------write chunk info-------------------------------*/
    std::unique_ptr<unsigned char[]> buffer{ new unsigned char[asset.m_info.baseSize] };
    
    auto const& chunkData = asset.m_chunkData;
	auto propCount = chunkData.propCount;
    //write prop count
    memcpy(buffer.get(), &chunkData.propCount, sizeof(unsigned int));
    //write props
    memcpy(buffer.get() + sizeof(unsigned int), chunkData.props, sizeof(unsigned int) * asset.m_chunkData.propCount);
    //write raw data
    memcpy(buffer.get() + (propCount + 1) * sizeof(unsigned int), chunkData.raw, chunkData.props[0]);

    m_file.write(reinterpret_cast<const char*>(buffer.get()), asset.m_info.baseSize);
    /*--------------------------------------------------------------------------*/
}

void AssetBundle::WriteToFile(const char* filepath)
{
    std::ofstream m_file(filepath, std::ios::binary);
    if (!m_file.is_open())
        throw std::runtime_error("Failed to open file");
    m_file.write(reinterpret_cast<const char*>(&m_header), sizeof(m_header));

    for (auto const& asset : m_assets)
    {
        WriteAssetToFile(m_file, asset);
    }
}
    
    


}//namespace RRES