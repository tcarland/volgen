/** @file DirNode.hpp
  *
  * DirNode objects are the parents of DirNode and FileNode objects
  * that represent our filesystem tree. DirNode is the core object
  * of the HeirarchicalStringTree container managed by VolGen.

  * Copyright (c) 2009-2025 Timothy C. Arland <tcarland@gmail.com>
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
#ifndef _VOLGEN_DIRNODE_HPP_
#define _VOLGEN_DIRNODE_HPP_

#include "FileNode.hpp"


namespace volgen {


class DirNode;

#define VOLGEN_NODESIZE   4096


/**  DirNode represents a filesystem directory node within
  *  our custom DirectoryTree (the DirTree object). Each
  *  instance holds a list of 'FileNode' assets for this
  *  directory level only, as the tree maintains the directory
  *  structure of DirNodes.
 **/
class DirNode {

  public:

    DirNode() : dnodesz(VOLGEN_NODESIZE) {}

    uint64_t getFileSize() const
    {
        uint64_t tsz = 0;
        FileNodeSet::const_iterator aIter;
        for ( aIter = files.begin(); aIter != files.end(); ++aIter ) {
            if ( ! aIter->symlink )
                tsz += aIter->getFileSize();
        }
        return tsz;
    }

    uint64_t getDiskSize() const
    {
        uint64_t tsz = 0;
        FileNodeSet::const_iterator aIter;
        for ( aIter = files.begin(); aIter != files.end(); ++aIter ) {
            if ( ! aIter->symlink )
                tsz += aIter->getDiskSize();
        }
        tsz += dnodesz;
        return tsz;
    }

    uint64_t getPhySize() const   { return this->getDiskSize(); }
    uint64_t getBlockSize() const { return this->getDiskSize(); }

    uint32_t getFileCount() const
    {
        return this->files.size();
    }

    void     setNodeSize ( uint32_t sz )
    {
        dnodesz = sz;
    }

  public:

    FileNodeSet  files;
    uint32_t     dnodesz;

};

}  // namespace

#endif  // _VOLGEN_DIRNODE_HPP_
