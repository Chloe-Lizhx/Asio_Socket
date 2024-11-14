#pragma once

#include <boost/uuid/name_generator.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <algorithm>
#include <boost/algorithm/string/trim.hpp>
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string_view>
#include <iostream>
#include "utils/assertion.hpp"
namespace com{
namespace fs = std::filesystem;

class conInfo{
public:
    conInfo(std::string_view acceptorName,
            std::string_view requesterName,
            std::string_view tag,
            int rank ,
            std::string_view addressDirectory ) noexcept
            :acceptorName(acceptorName),
             requesterName(requesterName),
             tag(tag),
             rank(rank),
             addressDirectory(addressDirectory){}

    conInfo(std::string_view acceptorName,
            std::string_view requesterName,
            std::string_view tag,
            std::string_view addressDirectory ) noexcept
            :acceptorName(acceptorName),
             requesterName(requesterName),
             tag(tag),
             addressDirectory(addressDirectory){}
protected:
    std::string const acceptorName;
    std::string const requesterName;
    std::string const tag;
    int const rank = -1;
    std::string const addressDirectory;
    //根据字符串生成唯一的uuid
    std::string comHash(std::string_view s) const ;
    //根据[accpetor,requester,tag,rank]的uuid生成文件路径，比如e9/61a7dbe33b574c8490ae23833a44ae，
    std::string HashedFilePath(std::string_view acceptorName,std::string_view requesterName,std::string_view tag,int rank) const;
    //根据accpetor,requester，addressDirectory得到本地的路径，./com-run/lzx-zxl
    std::string getLocalDirectory(std::string_view acceptorName,std::string_view requesterName, std::string_view addressDirectory) const;
    //将fliepath和localdirectory合并：./com-run/lzx-zxl/e9/61a7dbe33b574c8490ae23833a44ae
    std::string getFileName() const;
};

class conInfoReader:public conInfo
{
public:
    conInfoReader(std::string_view acceptorName,
                  std::string_view requesterName,
                  std::string_view tag,
                  int              rank,
                  std::string_view addressDirectory) noexcept
                  : conInfo(acceptorName, requesterName, tag, rank, addressDirectory){}

    conInfoReader(std::string_view acceptorName,
                  std::string_view requesterName,
                  std::string_view tag,
                  std::string_view addressDirectory) noexcept
                  : conInfo(acceptorName, requesterName, tag, addressDirectory){}
    std::string read() const;

};

class conInfoWriter:public conInfo
{
public :
    conInfoWriter(std::string_view acceptorName,
                  std::string_view requesterName,
                  std::string_view tag,
                  int              rank,
                  std::string_view addressDirectory) noexcept
                 : conInfo(acceptorName, requesterName, tag, rank, addressDirectory){}

    conInfoWriter(std::string_view acceptorName,
                  std::string_view requesterName,
                  std::string_view tag,
                  std::string_view addressDirectory) noexcept
                  : conInfo(acceptorName, requesterName, tag, addressDirectory){}

  /// 移除有关连接的文件，文件夹./com_run/hash 为空
  ~conInfoWriter();

  void write(std::string_view info) const;
};

std::string conInfo::comHash(std::string_view s) const
{
try {
  boost::uuids::string_generator ns_gen;
  auto                           ns = ns_gen("af7ce8f2-a9ee-46cb-38ee-71c318aa3580"); // precice.org的md5 hash值 

  boost::uuids::name_generator gen{ns};//构建名字生成器
  return boost::uuids::to_string(gen(s.data(), s.size()));//为s生成uuid,并返回uuid的字符串

} catch (const std::exception &e) {

    return "";
    }
}

std::string conInfo::HashedFilePath(std::string_view acceptorName,std::string_view requesterName,std::string_view tag,int rank) const
{
  constexpr int     firstLevelLen = 2;
  std::string const s             = std::string(acceptorName).append(tag).append(requesterName).append(std::to_string(rank));
  std::string       hash          = comHash(s);
  hash.erase(std::remove(hash.begin(), hash.end(), '-'), hash.end());//输出没有“—”的uuid

  auto p = fs::path(hash.substr(0, firstLevelLen)) / hash.substr(firstLevelLen);

  return p.string(); 
}

std::string conInfo::getLocalDirectory(std::string_view acceptorName,std::string_view requesterName, std::string_view addressDirectory) const
{
  std::string directional = std::string(acceptorName).append("-").append(requesterName);

  auto p = fs::path(addressDirectory.begin(), addressDirectory.end()) / "com-run" / directional;

  return p.string();
}

std::string conInfo::getFileName() const 
{
    auto hashed = HashedFilePath(acceptorName,requesterName,tag,rank);
    auto localDirectory = getLocalDirectory(acceptorName,requesterName,addressDirectory);
    auto filename = fs::path(localDirectory) / hashed;
    return filename;
}

std::string conInfoReader::read() const
{
    auto path = getFileName();
    if(!fs::exists(path)){std::cerr<<"comInfoReader::read can't get filename"<<std::endl;}
    std::ifstream ifs(path);
    std::string addressData;
    std::getline(ifs,addressData);
    //去掉字符串末尾空白字符
    boost::algorithm::trim_right(addressData);
    return addressData;
}

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
    Assert(fs::exists(path),"通信文件没有建立成功");
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
