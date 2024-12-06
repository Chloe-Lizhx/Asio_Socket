#include <thread>
#include "connectionInfo.hpp"

namespace com{
namespace impl
{
std::string comHash(std::string_view s) 
{
try {
  boost::uuids::string_generator ns_gen;
  auto                           ns = ns_gen("af7ce8f2-a9ee-46cb-38ee-71c318aa3580"); // precice.org的md5 hash值 

  boost::uuids::name_generator gen{ns};//构建名字生成器
  return boost::uuids::to_string(gen(s.data(), s.size()));//为s生成uuid,并返回uuid的字符串

} catch (const std::exception &e) 
{
    return "";
    }
}
//根据[accpetor,requester,tag,rank]的uuid生成文件路径，比如e9/61a7dbe33b574c8490ae23833a44ae，
std::string HashedFilePath(std::string_view acceptorName,std::string_view requesterName,std::string_view tag,int rank)
{
  constexpr int     firstLevelLen = 2;
  std::string const s             = std::string(acceptorName).append(tag).append(requesterName).append(std::to_string(rank));
  std::string       hash          = comHash(s);
  hash.erase(std::remove(hash.begin(), hash.end(), '-'), hash.end());//输出没有“—”的uuid

  auto p = fs::path(hash.substr(0, firstLevelLen)) / hash.substr(firstLevelLen);

  return p.string(); 
}
/// @brief 得到地址发布信息的所在文件夹 addressDirectory/"com-run"/acceptorName-requesterName/
std::string getLocalDirectory(std::string_view acceptorName,std::string_view requesterName, std::string_view addressDirectory)
{
  std::string directional = std::string(acceptorName).append("-").append(requesterName);

  auto p = fs::path(addressDirectory.begin(), addressDirectory.end()) / "com-run" / directional;

  return p.string();
}
}//namespace impl
std::string conInfo::getFileName() const 
{
    auto hashed = impl::HashedFilePath(acceptorName,requesterName,tag,rank);
    auto localDirectory = impl::getLocalDirectory(acceptorName,requesterName,addressDirectory);
    auto filename = fs::path(localDirectory) / hashed;
    return filename;
}
/// @brief 读取地址
std::string conInfoReader::read() const
{
    auto path = getFileName();
    const auto waitdelay = std::chrono::milliseconds(1);
    while(!fs::exists(path))
    {std::this_thread::sleep_for(waitdelay);}
    std::ifstream ifs(path);
    std::string addressData;
    std::getline(ifs,addressData);
    //去掉字符串末尾空白字符
    boost::algorithm::trim_right(addressData);
    return addressData;
}
/// @brief 创建通信文件，并将地址信息写入
void conInfoWriter::write(std::string_view info) const
{
    auto path = getFileName();
    auto tmp = fs::path(path+"~");
    //创建文件，./com-run/lzx-zxl/e9/61a7dbe33b574c8490ae23833a44ae~
    //tmp.parent_path()=./com-run/lzx-zxl/e9
    fs::create_directories(tmp.parent_path());
    {
    std::ofstream ofs(tmp.string());
    std::ostringstream oss;
    oss << info << "\n";
    oss << "Acceptor: " << acceptorName << ", ";
    oss << "Requester: " << requesterName << ", ";
    oss << "Tag: " << tag << ", ";
    oss << "Rank: " << rank;
    ofs<<oss.str();
    }
    Assert(!fs::exists(tmp),"临时文件没有创建成功");
    //对tmp所指代的文件进行覆盖，但是tmp本身不发生变化
    fs::rename(tmp,path);
    Assert(!fs::exists(path),"通信文件没有建立成功");
}

conInfoWriter::~conInfoWriter()
{
    auto path = getFileName();
    Assert(!fs::exists(path),"conInfoWriter析构失败,由于通信文件已经不存在");
    try
    {
        //移除文件
        fs::remove(path);
        Assert(fs::exists(path),"conInfoWriter析构失败,由于通信文件没有被成功的remove");
    }
    catch(const fs::filesystem_error& e)
    {
        std::cerr << e.what() << '\n';
    }
    
}
}