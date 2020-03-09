/**
  * @file   FileNode.hpp
  * @author tcarland@gmail.com
  *
  * Copyright (c) 2009-2012,2020 Timothy C. Arland <tcarland@gmail.com>
  *
  * Volgen is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * Volgen is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with Volgen.  If not, see <https://www.gnu.org/licenses/>.
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
