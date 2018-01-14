/**
  *  An application for generating volumes of a given size, based
  *  on a file system directory.
  *  Intended for performing backups to various media types 
  *  such as DVD-RW, CD-RW, USB Flash, etc.
  *
  * @file   VolGen.h 
  * @author tcarland@gmail.com
  *
  * Copyright (c) 2009,2010 Timothy C. Arland <tcarland@gmail.com>
  *
 **/
#ifndef _VOLGEN_VOLGEN_H_
#define _VOLGEN_VOLGEN_H_

#include <inttypes.h>
#include <sys/types.h>


#include "FileNode.hpp"
#include "DirNode.hpp"

#include "HeirarchicalStringTree.hpp"
using namespace tcanetpp;


namespace volgen {

#define VOLGEN_VERSION       "v0.304"
#define VOLGEN_LICENSE       "Copyright (c)2009-2013 Timothy C. Arland <tcarland@gmail.com>"

#define VOLGEN_ARCHIVEDIR    ".volgen"
#define VOLGEN_DEFAULT_NAME  "Volume_"
#define VOLGEN_VOLUME_MB     4400
#define VOLGEN_BLOCKSIZE     512


typedef tcanetpp::HeirarchicalStringTree<DirNode>  DirTree;


struct VolumeItem {
    std::string  fullname;
    std::string  name;
    uint64_t     size;
    float        vratio;

    VolumeItem() : size(0), vratio(0.0) {}
};

typedef std::list<VolumeItem> ItemList;


struct Volume {
    std::string  name;
    ItemList     items;
    uint64_t     size;
    float        vtotal;

    Volume() : size(0), vtotal(0.0) {}

    Volume ( const std::string & vname ) 
        : name(vname),
          size(0), 
          vtotal(0.0) 
    {}

    bool operator< ( const Volume & v ) const 
    {
        return(name < v.name);
    }

    bool operator== ( const Volume & v ) const
    {
        if ( name.compare(v.name) == 0 )
            return true;
        return false;
    }
};

typedef std::list<Volume*> VolumeList;



class VolGen {

  public:

    VolGen ( const std::string & path );
    ~VolGen();

    bool     read();

    void     displayTree();

    void     createVolumes();
    void     displayVolumes ( bool show = false );

    void     generateVolumes ( const std::string & volpath );
    uint64_t getDirSize      ( const std::string & path );

    void     setVolumeSize   ( size_t volsz );
    size_t   getVolumeSize() const;

    void     setBlockSize    ( size_t blksz );
    size_t   getBlockSize() const;

    void     setDebug ( bool d );

    static std::string  GetCurrentPath();
    static std::string  GetVolumeName ( size_t sz );


  private:

    bool     readDirectory ( const std::string & path );
    void     createVolumes ( const std::string & path );


  private:

    DirTree             _dtree;
    VolumeList          _vols;
    Volume*             _curv;

    std::string         _path;

    size_t              _volsz;
    size_t              _blksz;
    bool                _debug;

};


}  // namespace

#endif  // _VOLGEN_VOLGEN_H_
