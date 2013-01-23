/**
  * @file include/FileNode.hpp 
  * @author tcarland@gmail.com
  * 
 **/
#ifndef _VOLGEN_FILENODE_HPP_
#define _VOLGEN_FILENODE_HPP_

#include <inttypes.h>

#include <string>
#include <set>

#include "StringUtils.h"
using namespace tcanetpp;


namespace volgen {

class FileNode;
typedef std::set<FileNode>  AssetSet;


/**  Represents a filesytem filenode and its associated file with 
  *  filesize and blocksize attributes
 **/
class FileNode {
  public:

    FileNode ( const std::string & filename, uint64_t sz, uint64_t blksz = 0 )
        : fileName(filename),
          fileSize(sz),
          blockSize(blksz),
          symlink(false)
    {}

    bool operator< ( const FileNode & f ) const
    {
        return(fileName < f.getFileName());
    }

    const
    std::string&  getFileName()  const  { return fileName; }
    uint64_t      getFileSize()  const  { return fileSize; }
    uint64_t      getDiskSize()  const  { return blockSize; }
    uint64_t      getBlockSize() const  { return blockSize; }

    //void          statFileInfo();
    static std::string GetNameOnly ( const std::string & fname )
    {
        std::string name;
        int indx  = -1;
        
        indx = StringUtils::lastIndexOf(fname, "/");
        name = fname.substr(indx+1);

        return name;
    }
 
  public:
    std::string   fileName;
    uint64_t      fileSize;
    uint64_t      blockSize;
    bool          symlink;

    // sync time?
};


}  // namespace


#endif  // _VOLGEN_FILENODE_HPP_
