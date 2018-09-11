/**
  * @file   FileNode.hpp
  * @author tcarland@gmail.com
  *
  * Copyright (c) 2009,2010 Timothy C. Arland <tcarland@gmail.com>
  *
  *
 **/
#ifndef _VOLGEN_FILENODE_HPP_
#define _VOLGEN_FILENODE_HPP_

#include <inttypes.h>

#include <string>
#include <set>

#include "util/StringUtils.h"
using namespace tcanetpp;


namespace volgen {


class FileNode;
typedef std::set<FileNode>  FileNodeSet;


/**  Represents a filesytem filenode; the associated filename with
  *  filesize and blocksize attributes.
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

    bool operator== ( const FileNode & f ) const
    {
        if ( this->fileName.compare(f.fileName) == 0 )
            return true;
        return false;
    }

    const
    std::string&  getFileName()  const { return fileName; }
    uint64_t      getFileSize()  const { return fileSize; }
    uint64_t      getDiskSize()  const { return blockSize; }
    uint64_t      getBlockSize() const { return blockSize; }


    static std::string GetNameOnly ( const std::string & fname )
    {
        std::string name;
        int indx  = -1;

        indx = StringUtils::LastIndexOf(fname, "/");
        name = fname.substr(indx+1);

        return name;
    }

  public:

    std::string   fileName;
    uint64_t      fileSize;
    uint64_t      blockSize;
    bool          symlink;

};


}  // namespace


#endif  // _VOLGEN_FILENODE_HPP_
