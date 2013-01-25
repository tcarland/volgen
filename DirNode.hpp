/**
  * @file include/DirNode.hpp 
  * @author 
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
  *  directory level only as the tree maintains the directory 
  *  structure.
 **/
class DirNode {

  public:

    DirNode() : dnodesz(VOLGEN_NODESIZE) {}

    uint64_t getFileSize() const
    {
        uint64_t tsz = 0;
        AssetSet::const_iterator aIter;
        for ( aIter = files.begin(); aIter != files.end(); ++aIter ) {
            if ( ! aIter->symlink )
                tsz += aIter->getFileSize();
        }
        return tsz;
    }

    uint64_t getDiskSize() const
    {
        uint64_t tsz = 0;
        AssetSet::const_iterator aIter;
        for ( aIter = files.begin(); aIter != files.end(); ++aIter ) {
            if ( ! aIter->symlink )
                tsz += aIter->getDiskSize();
        }
        tsz += dnodesz;
        return tsz;
    }

    uint32_t getFileCount() const
    {
        return this->files.size();
    }

  public:

    AssetSet  files;
    uint32_t  dnodesz;

};


}  // namespace


#endif  // _VOLGEN_DIRNODE_HPP_

