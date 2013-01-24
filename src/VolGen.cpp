#define _VOLGEN_VOLGEN_CPP_

extern "C" {
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
}

#include <sys/stat.h>

#include <iostream>
#include <iomanip>


#include "VolGen.h"


namespace volgen {



// -------------------------------------------------------------- //
// DirTree Predicates

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


bool
VolGen::read()
{
    return this->readDirectory(_path);
}

void
VolGen::createVolumes()
{
    return this->createVolumes(_path);
}

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
            bltotal += (lsb.st_blocks * VOLGEN_BLOCKSIZE);
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
            blks = (fsb.st_blocks * VOLGEN_BLOCKSIZE);
            node = _dtree.find(dname);

            if ( node == NULL ) 
            {
                DirTree::BranchNodeList  branches;
                node = _dtree.insert(dname, std::inserter(branches, branches.begin()));
                if ( node == NULL ) {
                    std::cout << "Failed to insert path into DirTree " << std::endl;
                    return false;
                }
            }
            if ( _debug ) 
                std::cout << " d> '" << dname << "/' : " << blks << " (" << size << ")" << std::endl;

            this->readDirectory(dname);

            continue;
        } 
        else 
        {
            size = fsb.st_size;
            blks = (fsb.st_blocks * VOLGEN_BLOCKSIZE);
            node = _dtree.find(path);

            if ( node == NULL ) {
                DirTree::BranchNodeList  branches;
                node = _dtree.insert(path, std::inserter(branches, branches.begin()));
                if ( node == NULL ) {
                    std::cout << "Failed to find path in DirTree " << path << std::endl;
                    return false;
                } else if ( _debug ) {
                    std::cout << "added path '" << path << "' to DirTree" << std::endl;
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
                    std:: cout << " l> '";
                else
                    std::cout << " f> '";
                std::cout << fn.getFileName() << "' : " << fn.fileSize 
                    << " blocksize: " << blks << std::endl;
            }
        }
    }

    if ( _debug ) {
        std::cout << "Total File sizes: " << (bytotal/1024) << " Kbytes. Blocks: "
            << (bltotal/1024) << std::endl;
    }

    return true;
}


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


void
VolGen::createVolumes ( const std::string & path )
{
    DirTree::Node * node = _dtree.find(path);

    if ( node == NULL ) {
        std::cout << "volgen::createVolumes() Error finding path: " << path << std::endl;
        return;
    }

    Volume * v = NULL;

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
        float vol = (dmb / _volsz) * 100.0;

        if ( dirsize.fsize == 0 )
            continue;
    
        if ( vol > 95.0 ) {
            //_vols.push_back(v);
            std::string dirstr = "/" + nIter->second->getAbsoluteName();
            this->createVolumes(dirstr);
            continue;
        }

        v = _curv;

        VolumeItem item;
        item.fullname = "/" + nIter->second->getAbsoluteName();
        item.name     = nIter->second->getName();
        item.size     = dmb;
        item.vrat     = vol;

        if ( (v->vtot + vol) > 95.0 ) {
            v     = new Volume(VolGen::GetVolumeName(_vols.size()));
            _curv = v;
            _vols.push_back(v);
        }

        v->size += item.size;
        v->vtot += item.vrat;
        v->items.push_back(item);
    }

    AssetSet & assets = node->getValue().files;
    AssetSet::iterator  fIter;

    for ( fIter = assets.begin(); fIter != assets.end(); ++fIter )
    {
        const FileNode & file = *fIter;
        VolumeItem       item;

        float fmb = (file.getDiskSize() / (1024 * 1024));
        float vol = (fmb / _volsz) * 100.0;

        if ( vol > 95.0 ) {
            std::cout << "WARNING: File is larger than volume size, skipping file: " 
                << file.getFileName() << std::endl;
            continue;
        }

        v = _curv;

        item.fullname = file.getFileName();
        item.name     = FileNode::GetNameOnly(item.fullname);
        item.size     = fmb;
        item.vrat     = vol;

        if ( (v->vtot + vol) > 95.0 ) {
            v     = new Volume(VolGen::GetVolumeName(_vols.size()));
            _curv = v;
            _vols.push_back(v);
        }

        v->size += item.size;
        v->vtot += item.vrat;
        v->items.push_back(item);
    }

    return;
}


void 
VolGen::displayVolumes ( bool show )
{
    VolumeList::iterator vIter;
    
    std::cout << "Number of volumes = " << _vols.size() << std::endl;

    for ( vIter = _vols.begin(); vIter != _vols.end(); ++vIter )
    {
        Volume * v = (Volume*) *vIter;
        std::cout << v->name << " : " << v->size << " Mb : " 
            << v->vtot << "% : " << (v->items.size() + 1) 
            << " item(s)" << std::endl;
        
        if ( show ) {
            ItemList::iterator iIter;
            for ( iIter = v->items.begin(); iIter != v->items.end(); ++iIter ) 
            {
                std::cout << "   " << iIter->name << " : " << iIter->size << " Mb : "
                    << iIter->vrat << " %" << std::endl;
            }
        }
    }
    std::cout << std::endl;

    return;
}


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

        //std::cout << vol.name << " = " << newpath << std::endl;

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


void
VolGen::setVolumeSize ( size_t volsz )
{
    _volsz = volsz;
}

size_t
VolGen::getVolumeSize() const
{
    return _volsz;
}

void
VolGen::setDebug ( bool d )
{
    _debug = d;
}


std::string 
VolGen::GetVolumeName ( size_t volsz )
{
    std::ostringstream  vol;
    vol << "Volume_" << std::setfill('0') << std::setw(2) << (volsz + 1);
    return vol.str();
}



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
