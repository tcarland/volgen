/**
  * @file   VolGen.cpp
  * @author tcarland@gmail.com
  *
  * Copyright (c) 2009,2010 Timothy C. Arland <tcarland@gmail.com>
  *
 **/
#define _VOLGEN_VOLGEN_CPP_

extern "C" {
#include <unistd.h>
#include <dirent.h>
}

#include <sys/stat.h>
#include <iostream>
#include <iomanip>

#include "VolGen.h"



namespace volgen {


// -------------------------------------------------------------- //
// DirTree Predicates

/**  Predicate for accumulating the total size of a directory */
struct DirSizePredicate {
    uint64_t  fsize, dsize, dcount, fcount;

    DirSizePredicate() 
      : fsize(0), dsize(0), dcount(0), fcount(0) 
    {}

    void operator() ( const DirTree::Node * node ) 
    {
        fsize  += node->getValue().getFileSize();
        dsize  += node->getValue().getDiskSize();
        fcount += node->getValue().getFileCount();
        dcount++;
    }
};

/** Predicate for displaying the size of a directory tree */
struct PrintTreePredicate {
    DirTree     * tree;
    std::string   rootpath;
    std::string   disppath;


    explicit PrintTreePredicate ( DirTree           * dtree, 
                                  const std::string & rootPath,
                                  const std::string & dispPath = "" )
        : tree(dtree),
          rootpath(rootPath),
          disppath(dispPath)
    {
        std::cout << std::endl << rootpath << ":" << std::endl << std::endl; 
        std::cout << std::setw(20) << std::setiosflags(std::ios_base::left) << "Size (Kb) "
                  << std::setw(18) << "Size (Mb)"
                  << std::setw(8)  << "D/F"
                  << " Directory" 
                  << std::endl;
        std::cout << std::setw(20) << std::setiosflags(std::ios_base::left) << "----------"
                  << std::setw(18) << "--------"
                  << std::setw(8)  << "------"
                  << "----------------------" 
                  << std::endl;
    }


    void operator() ( DirTree::Node * node )
    {
        DirSizePredicate dirSz;
        tree->depthFirstTraversal(node, dirSz);

        std::string name = "/";
        name.append(node->getAbsoluteName());

        if ( StringUtils::startsWith(name, rootpath) )
            name = name.substr(rootpath.length());
        if ( StringUtils::startsWith(name, "/") )
            name = name.substr(1);
        if ( name.empty() )
            name = disppath;

        std::ostringstream cnts;
        cnts << node->getChildren().size() << "/" << node->getValue().getFileCount();

        float sz  = ((float)dirSz.dsize / 1024);
        float dmb = ((float)dirSz.dsize / (1024 * 1024));

        std::cout << std::setw(20) << std::setiosflags(std::ios_base::left);
        if ( sz < 100.0 )
            std::cout << std::setprecision(2) << std::fixed << sz;
        else
            std::cout << ((int)sz);

        std::cout << std::setw(18);
        if ( dmb < 100.0 )
            std::cout << std::setprecision(3) << std::fixed << dmb;
        else
            std::cout << ((int)dmb);

        std::cout << std::setw(8)  << cnts.str()
                  << name 
                  << std::endl;
    }
};


// -------------------------------------------------------------- //


VolGen::VolGen ( const std::string & path )
    : _curv(NULL),
      _path(path),
      _volsz(VOLGEN_VOLUME_MB),
      _blksz(VOLGEN_BLOCKSIZE),
      _debug(false)
{
}

VolGen::~VolGen()
{
    VolumeList::iterator vIter;

    for ( vIter = _vols.begin(); vIter != _vols.end(); ++vIter )
        delete *vIter;
    _vols.clear();
}


/**  Reads and parses the given root directory building a tree of the 
  *  underlying directory structure.
 **/
bool
VolGen::read()
{
    return this->readDirectory(_path);
}

/**  Creates a list of Volumes from the directory tree. */
void
VolGen::createVolumes()
{
    return this->createVolumes(_path);
}

/**  Used for recursively walking the directory tree */
bool
VolGen::readDirectory ( const std::string & path )
{
    DIR*           dirp;
    struct dirent* dire;
    struct stat    fsb, lsb;
    std::string    dname;

    uint64_t       size    = 0;
    uint64_t       blks    = 0;
    uint64_t       bytotal = 0;
    uint64_t       bltotal = 4096;
    bool           link    = false;
    bool           result  = true;

    DirTree::Node * node = NULL;

    if ( _debug ) 
        std::cout << "VolGen::readDirectory() " << path << std::endl;

    if ( (dirp = ::opendir(path.c_str())) == NULL )
        return false;

    while ( (dire = ::readdir(dirp)) != NULL )
    {
        link  = false;
        dname = dire->d_name;

        if ( dname.compare(".") == 0 || dname.compare("..") == 0 )
            continue;

        dname = path + "/" + dname;

        if ( ::lstat(dname.c_str(), &lsb) < 0 ) {
            std::cout << "lstat() failed for '" << dname << "'" << std::endl;
            continue;
        }

        if ( S_ISLNK(lsb.st_mode) ) {
            bltotal += (lsb.st_blocks * _blksz);
            link = true;
            if ( _debug ) 
                std::cout << " l> " << dname << std::endl;
        }

        if ( ::stat(dname.c_str(), &fsb) < 0 ) {
            std::cout << "stat() failed for '" << dname << "'" << std::endl;
            continue;
        }

        if ( ! link && S_ISDIR(fsb.st_mode) ) 
        {
            size = fsb.st_size;
            blks = (fsb.st_blocks * _blksz);
            node = _dtree.find(dname);

            if ( node == NULL ) 
            {
                DirTree::BranchNodeList  branches;
                node = _dtree.insert(dname, std::inserter(branches, branches.begin()));
                if ( node == NULL ) {
                    std::cout << "Failed to insert path into DirTree " << std::endl;
                    result = false;
                    break;
                }
            }

            if ( _debug ) 
                std::cout << " d> '" << dname << "/' : " << blks << " (" << size << ")" 
                          << std::endl;

            this->readDirectory(dname);

            continue;
        } 
        else 
        {
            size = fsb.st_size;
            blks = (fsb.st_blocks * _blksz);
            node = _dtree.find(path);

            if ( node == NULL ) {
                DirTree::BranchNodeList  branches;
                node = _dtree.insert(path, std::inserter(branches, branches.begin()));
                if ( node == NULL ) {
                    std::cout << "Failed to insert path in DirTree " << path << std::endl;
                    result = false;
                    break;
                } else if ( _debug ) {
                    std::cout << "  added path '" << path << "' to DirTree" << std::endl;
                }
            }

            FileNode  fn(dname, size, blks);
            fn.symlink  = link;
            DirNode & d = node->getValue();

            d.files.insert(fn);

            bytotal += size;
            bltotal += blks;
            
            if ( _debug ) 
            {
                if ( link )
                    std::cout << " l> '";
                else
                    std::cout << " f> '";
                std::cout << fn.getFileName() << "' : " << fn.fileSize 
                    << " blocksize: " << blks << std::endl;
            }
        }
    }
    ::closedir(dirp);

    if ( _debug ) {
        std::cout << "Total File sizes: " 
                  << std::setprecision(3) << (bytotal/1024) 
                  << " Kbytes. Blocks: " 
                  << std::setprecision(3) << (bltotal/1024) 
                  << std::endl;
    }

    return result;
}


/**  Displays the given directory tree and associated sizes */
void
VolGen::displayTree()
{
    PrintTreePredicate show(&_dtree, _path);
    
    if ( _path.empty() ) 
    {
        DirTree::NodeMap & nodemap = _dtree.getRoots();
        DirTree::NodeMapIter nIter;
        for ( nIter = nodemap.begin(); nIter != nodemap.end(); ++nIter )
            _dtree.depthFirstTraversal(nIter->second, show);
    } 
    else 
    {
        DirTree::Node * node = _dtree.find(_path);
        if ( node == NULL )
            return;
        _dtree.depthFirstTraversal(node, show);
    }
    std::cout << std::endl;
        
    return;
}


/**  Method for recursively walking the directory and file structure
  *  for building the Volume list.
 **/
void
VolGen::createVolumes ( const std::string & path )
{
    DirTree::Node * node = _dtree.find(path);

    if ( node == NULL ) {
        std::cout << "volgen::createVolumes() Error finding the root path: " 
            << path << std::endl;
        return;
    }

    Volume * vol = NULL;

    if ( _curv == NULL ) {
        _curv = new Volume(VolGen::GetVolumeName(_vols.size()));
        _vols.push_back(_curv);
    }

    DirTree::NodeMap & nodemap = node->getChildren();
    DirTree::NodeMapIter nIter;

    for ( nIter = nodemap.begin(); nIter != nodemap.end(); ++nIter )
    {
        DirSizePredicate dirsize;

        _dtree.depthFirstTraversal(nIter->second, dirsize);

        float dmb = (dirsize.dsize / (1024 * 1024));
        float vrt = (dmb / _volsz) * 100.0;

        if ( dirsize.fsize == 0 )
            continue;
    
        if ( vrt > 95.0 ) {
            std::string dirstr = "/" + nIter->second->getAbsoluteName();
            this->createVolumes(dirstr);
            continue;
        }

        vol = _curv;

        VolumeItem  item;
        item.fullname = "/" + nIter->second->getAbsoluteName();
        item.name     = nIter->second->getName();
        item.size     = dmb;
        item.vratio   = vrt;

        if ( (vol->vtotal + vrt) > 95.0 ) {
            vol   = new Volume(VolGen::GetVolumeName(_vols.size()));
            _curv = vol;
            _vols.push_back(vol);
        }

        vol->size   += item.size;
        vol->vtotal += item.vratio;
        vol->items.push_back(item);
    }

    FileNodeSet & assets = node->getValue().files;
    FileNodeSet::iterator  fIter;

    for ( fIter = assets.begin(); fIter != assets.end(); ++fIter )
    {
        const FileNode & file = *fIter;
        VolumeItem       item;

        float fmb = (file.getDiskSize() / (1024 * 1024));
        float vrt = (fmb / _volsz) * 100.0;

        if ( vrt > 95.0 ) {
            std::cout << "WARNING: File is larger than volume size, skipping file: " 
                << file.getFileName() << std::endl;
            continue;
        }

        vol = _curv;

        item.fullname = file.getFileName();
        item.name     = FileNode::GetNameOnly(item.fullname);
        item.size     = fmb;
        item.vratio   = vrt;

        if ( (vol->vtotal + vrt) > 95.0 ) {
            vol   = new Volume(VolGen::GetVolumeName(_vols.size()));
            _curv = vol;
            _vols.push_back(vol);
        }

        vol->size   += item.size;
        vol->vtotal += item.vratio;
        vol->items.push_back(item);
    }

    return;
}


/**  Displays the created Volume list */
void 
VolGen::displayVolumes ( bool show )
{
    VolumeList::iterator vIter;
    
    std::cout << "Number of volumes = " << _vols.size() << std::endl;

    for ( vIter = _vols.begin(); vIter != _vols.end(); ++vIter )
    {
        Volume * vol = (Volume*) *vIter;
        std::cout << vol->name << " : " << vol->size << " Mb : " 
            << vol->vtotal << "% : " << vol->items.size() 
            << " item(s)" << std::endl;
        
        if ( show ) {
            ItemList::iterator iIter;
            for ( iIter = vol->items.begin(); iIter != vol->items.end(); ++iIter ) 
            {
                std::cout << "   " << iIter->name << " : " << iIter->size << " Mb : "
                    << std::setprecision(3) << iIter->vratio << " %" << std::endl;
            }
        }
    }
    std::cout << std::endl;

    return;
}


/**  Generates the volume linkage in the given path */
void 
VolGen::generateVolumes ( const std::string & volpath )
{
    VolumeList::iterator vIter;

    std::string newpath;

    for ( vIter = _vols.begin(); vIter != _vols.end(); ++vIter )
    {
        Volume * vol = *vIter;
        newpath      = volpath;
        newpath.append("/").append(vol->name);
        newpath.append("/");

        struct stat sb;
        if ( ::stat(newpath.c_str(), &sb) != 0 ) 
        {
            if ( errno == EACCES ) {
                std::cout << "Error in volgen path!" << std::endl;
                return;
            }
            if ( ::mkdir(newpath.c_str(), S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) < 0 ) {
                std::cout << "Error in mkdir '" << newpath << "' : " 
                    << strerror(errno) << std::endl;
                return;
            }
        }

        ItemList::iterator iIter;
        for ( iIter = vol->items.begin(); iIter != vol->items.end(); ++iIter ) 
        {
            VolumeItem & item = *iIter;
            std::string slink = newpath;
            slink.append(item.name);

            int r = ::symlink(item.fullname.c_str(), slink.c_str());

            if ( r != 0 ) {
                std::cout << "Error in symlink: " << slink 
                    << " : " << strerror(errno) << std::endl;
            }
        }
    }

    std::cout << "Volumes generated in " << volpath << std::endl;

    return;
}


/** Determines the size of the give directory */
uint64_t 
VolGen::getDirSize ( const std::string & path )
{
    DirTree::Node * node = _dtree.find(path);
    if ( node == NULL ) 
        return 0;

    DirSizePredicate  dirsize;
    _dtree.depthFirstTraversal(node, dirsize);

    return dirsize.dsize;
}


/**  Sets a fixed size of all volumes to be generated. */
void
VolGen::setVolumeSize ( size_t volsz )
{
    _volsz = volsz;
}


/**  Returns the configured volume size */
size_t
VolGen::getVolumeSize() const
{
    return _volsz;
}


/**  Sets the configured disk block size used for 
  *  calculating dir/file actual bytes consumed.
 **/
void
VolGen::setBlockSize ( size_t blksz )
{
    _blksz = blksz;
}


/**  Returns the configured block size. */
size_t
VolGen::getBlockSize() const
{
    return _blksz;
}


void
VolGen::setDebug ( bool d )
{
    _debug = d;
}


/** Creates a string of the next volume name in the list */
std::string 
VolGen::GetVolumeName ( size_t volsz )
{
    std::ostringstream  vol;
    vol << "Volume_" << std::setfill('0') << std::setw(2) << (volsz + 1);
    return vol.str();
}


/** Static function for determining the current working directory */
std::string 
VolGen::GetCurrentPath()
{
    std::string  path;
    char         pname[TCANET_MEDSTRLINE];
    size_t       psz = TCANET_MEDSTRLINE;

    if ( ::getcwd(&pname[0], psz) == NULL )
        return path;

    path.assign(pname);

    return path;
}


}  // namespace

// _VOLGEN_VOLGEN_CPP_
