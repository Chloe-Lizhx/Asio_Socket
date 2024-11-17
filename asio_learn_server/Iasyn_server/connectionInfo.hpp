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
namespace impl{
//根据字符串生成唯一的uuid
std::string comHash(std::string_view s) ;
//根据[accpetor,requester,tag,rank]的uuid生成文件路径，比如e9/61a7dbe33b574c8490ae23833a44ae，
std::string HashedFilePath(std::string_view acceptorName,std::string_view requesterName,std::string_view tag,int rank);
/// @brief 得到地址发布信息的所在文件夹 addressDirectory/"com-run"/acceptorName-requesterName/
std::string getLocalDirectory(std::string_view acceptorName,std::string_view requesterName, std::string_view addressDirectory);
}

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
    //将fliepath和localdirectory合并：./com-run/lzx-zxl/e9/61a7dbe33b574c8490ae23833a44ae
    std::string getFileName() const;
};
/// @brief 根据 acceptorName，requesterName，tag，rank(默认为 -1)，addressDirectory生成的地址文件读取要连接的IP+Port
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
/// @brief 根据 acceptorName，requesterName，tag，rank(默认为 -1），addressDirectory生成地址文件并写入IP+Port
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
}
