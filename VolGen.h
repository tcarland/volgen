/**
  *  An app for generating volumes of a given size of a file system directory.
  *  Intended for performing backups to various media types, 
  *  such as DVD-RW, CD-RW, USB Flash, etc.
  *
  * @file include/VolGen.h 
  * @author tcarland@gmail.com
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


#define VOLGEN_VERSION "v0.202"
#define VOLGEN_LICENSE "Copyright (c)2010-2013 Timothy C. Arland (tcarland@gmail.com)"

#define VOLGEN_ARCHIVEDIR       ".volgen"
#define VOLGEN_DEFAULT_SUFFIX   "-volume"
#define VOLGEN_VOLUME_MB        4400
#define VOLGEN_BLOCKSIZE        512



typedef tcanetpp::HeirarchicalStringTree<DirNode>  DirTree;



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

    void     setDebug ( bool d );

    static std::string  GetCurrentPath();
    static std::string  GetVolumeName ( size_t sz );


  protected:

    bool     readDirectory ( const std::string & path );
    void     createVolumes ( const std::string & path );


  private:

    DirTree     _dtree;
    VolumeList  _vols;
    Volume*     _curv;

    std::string _path;

    size_t      _volsz;
    bool        _debug;


};


}  // namespace


#endif  // _VOLGEN_VOLGEN_H_
