/**  volgen - Volume Generator
  *
  *    This is a proof of concept app for generating volumes 
  *  of a given size out of a file system directory.
  *  Intended for performing backups to small media types 
  *  such as DVD-RW, CD-RW, USB Flash, etc.
  *
  *  @Author  tcarland@gmail.com
  *  Mar. 7, 2011
  *  
 **/
#define _VOLGEN_MAIN_CPP_

extern "C" {
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
}

#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <string>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <set>


#include "StringUtils.h"
#include "FileUtils.h"
#include "HeirarchicalStringTree.hpp"
using namespace tcanetpp;


namespace volgen {

#define VOLGEN_VERSION "v0.133"
#define VOLGEN_LICENSE "Copyright (c)2010,2011 Timothy C. Arland (tcarland@gmail.com)"


#define VOLGEN_ARCHIVEDIR       ".volgen"
#define VOLGEN_DEFAULT_SUFFIX   "-volume"
#define VOLGEN_VOLUME_MB        4400
#define VOLGEN_NODESIZE         4096
#define VOLGEN_BLOCKSIZE        512


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


// FileNode containers
typedef std::map<std::string, FileNode>   AssetMap;
typedef std::set<FileNode>                AssetSet;
typedef std::list<FileNode>               AssetList;


/**  DirNode represents a filesystem directory node with 
  *  our custom DirectoryTree (the DirTree object). Each 
  *  instance holds a list of 'FileNode' assets for this 
  *  directory level only as our tree maintains the directory 
  *  structure.
 **/
struct DirNode {
    AssetSet  files;
    uint32_t  dnodesz;

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
};

// directory tree 
typedef tcanetpp::HeirarchicalStringTree<DirNode>  DirTree;


// -------------------------------------------------------------- //
// Volume objects

struct VolumeItem {
    std::string  fullname;
    std::string  name;
    uint64_t     size;
    float        vrat;

    VolumeItem() : size(0), vrat(0.0) {}
};
typedef std::list<VolumeItem> ItemList;

struct Volume {
    std::string  name;
    ItemList     items;
    uint64_t     size;
    float        vtot;

    Volume() : size(0), vtot(0.0) {}

    Volume ( const std::string & vname ) 
        : name(vname),
          size(0), 
          vtot(0.0) 
    {}
};
typedef std::list<Volume> VolumeList;


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

    explicit PrintTreePredicate ( DirTree * dtree, 
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
// volgen:: Functions

std::string getCurrentPath()
{
    std::string  path;
    char         pname[TCANET_MEDSTRLINE];
    size_t       psz = TCANET_MEDSTRLINE;

    if ( ::getcwd(&pname[0], psz) == NULL )
        return path;

    path.assign(pname);

    return path;
}

// TODO: Fix blocksize hardcoding
bool readDirectory ( const std::string & path, DirTree & dtree, long volsz, bool debug = false )
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

    if ( debug ) 
        std::cout << "volgen::readDirectory() " << path << std::endl;

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
            if ( debug ) 
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
            node = dtree.find(dname);

            if ( node == NULL ) 
            {
                DirTree::BranchNodeList  branches;
                node = dtree.insert(dname, std::inserter(branches, branches.begin()));
                if ( node == NULL ) {
                    std::cout << "Failed to insert path into DirTree " << std::endl;
                    return false;
                }
            }
            if ( debug ) 
                std::cout << " d> '" << dname << "/' : " << blks << " (" << size << ")" << std::endl;

            volgen::readDirectory(dname, dtree, volsz, debug);

            continue;
        } 
        else 
        {
            size = fsb.st_size;
            blks = (fsb.st_blocks * VOLGEN_BLOCKSIZE);
            node = dtree.find(path);

            if ( node == NULL ) {
                DirTree::BranchNodeList  branches;
                node = dtree.insert(path, std::inserter(branches, branches.begin()));
                if ( node == NULL ) {
                    std::cout << "Failed to find path in DirTree " << path << std::endl;
                    return false;
                } else if ( debug ) {
                    std::cout << "added path '" << path << "' to DirTree" << std::endl;
                }
            }

            FileNode  fn(dname, size, blks);
            fn.symlink  = link;
            DirNode & d = node->getValue();

            d.files.insert(fn);

            bytotal += size;
            bltotal += blks;
            
            if ( debug ) 
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

    if ( debug ) {
        std::cout << "Total File sizes: " << (bytotal/1024) << " Kbytes. Blocks: "
            << (bltotal/1024) << std::endl;
    }

    return true;
}


std::string getVolumeName ( size_t volsz )
{
    std::ostringstream  vol;
    vol << "Volume_" << std::setfill('0') << std::setw(2) << (volsz + 1);
    return vol.str();
}


uint64_t getDirSize ( const std::string & path, DirTree & dtree )
{
    DirTree::Node * node = dtree.find(path);
    if ( node == NULL ) 
        return 0;

    DirSizePredicate  dirsize;
    dtree.depthFirstTraversal(node, dirsize);

    return dirsize.dsize;
}


void displayTree ( const std::string & path, const std::string & target,
                   DirTree & dtree )
{
    PrintTreePredicate show(&dtree, path, target);
    
    if ( path.empty() ) 
    {
        DirTree::NodeMap & nodemap = dtree.getRoots();
        DirTree::NodeMapIter nIter;
        for ( nIter = nodemap.begin(); nIter != nodemap.end(); ++nIter )
            dtree.depthFirstTraversal(nIter->second, show);
    } 
    else 
    {
        DirTree::Node * node = dtree.find(path);
        if ( node == NULL )
            return;
        dtree.depthFirstTraversal(node, show);
    }
    std::cout << std::endl;
        
    return;
}

void createVolumes ( const std::string & path, DirTree & dtree, VolumeList & vols )
{
    DirTree::Node * node = dtree.find(path);

    if ( node == NULL ) {
        std::cout << "volgen::createVolumes() Error finding path: " << path << std::endl;
        return;
    }

    Volume  v;
    if ( vols.size() > 0 ) {
        v = vols.back();
        if ( v.vtot < 95.0 )
            vols.pop_back();
        else
            v = Volume(volgen::getVolumeName(vols.size()));
    } else {
        v = Volume(volgen::getVolumeName(vols.size()));
    }

    std::cout << v.name << ":" << std::endl;

    DirTree::NodeMap & nodemap = node->getChildren();
    DirTree::NodeMapIter nIter;

    for ( nIter = nodemap.begin(); nIter != nodemap.end(); ++nIter )
    {
        DirSizePredicate dirsize;

        dtree.depthFirstTraversal(nIter->second, dirsize);

        float dmb = (dirsize.dsize / (1024 * 1024));
        float vol = (dmb / 4400) * 100.0;               // FIX volume size

        if ( dirsize.fsize == 0 )
            continue;

        if ( vol > 95.0 ) {
            vols.push_back(v);
            std::string dirstr = "/" + nIter->second->getAbsoluteName();
            volgen::createVolumes(dirstr, dtree, vols);
            continue;
        }

        VolumeItem item;
        item.fullname = "/" + nIter->second->getAbsoluteName();
        item.name     = nIter->second->getName();
        item.size     = dmb;
        item.vrat     = vol;

        if ( (v.vtot+vol) > 95.0 ) {
            vols.push_back(v);
            v = Volume(volgen::getVolumeName(vols.size()));
            std::cout << v.name << ":" << std::endl;
        }

        v.size += item.size;
        v.vtot += item.vrat;
        v.items.push_back(item);
    }

    AssetSet & assets = node->getValue().files;
    AssetSet::iterator  fIter;

    for ( fIter = assets.begin(); fIter != assets.end(); ++fIter )
    {
        const FileNode & file = *fIter;
        VolumeItem       item;

        float fmb = (file.getDiskSize() / (1024 * 1024));
        float vol = (fmb / 4400) * 100.0;  // TODO: Volume size should not be hardcoded

        if ( vol > 95.0 ) {
            std::cout << "WARNING: File is larger than volume size, skipping file: " 
                << file.getFileName() << std::endl;
            continue;
        }

        item.fullname = file.getFileName();
        item.name     = FileNode::GetNameOnly(item.fullname);
        item.size     = fmb;
        item.vrat     = vol;

        if ( (v.vtot + vol) > 95.0 ) {
            vols.push_back(v);
            v = Volume(volgen::getVolumeName(vols.size()));
            std::cout << v.name << ":" << std::endl;
        }

        v.size += item.size;
        v.vtot += item.vrat;
        v.items.push_back(item);
    }

    if ( v.size > 0 )
        vols.push_back(v);

    return;
}

void displayVolumes ( VolumeList & vols, bool show = false )
{
    VolumeList::iterator vIter;
    
    std::cout << "Number of volumes = " << vols.size() << std::endl;

    for ( vIter = vols.begin(); vIter != vols.end(); ++vIter )
    {
        std::cout << vIter->name << " : " << vIter->size << " Mb : " 
            << vIter->vtot << "% : " << (vIter->items.size() + 1) 
            << " item(s)" << std::endl;
        
        if ( show ) {
            ItemList::iterator iIter;
            for ( iIter = vIter->items.begin(); iIter != vIter->items.end(); ++iIter ) 
            {
                std::cout << "   " << iIter->name << " : " << iIter->size << " Mb : "
                    << iIter->vrat << " %" << std::endl;
            }
        }
    }
    std::cout << std::endl;

    return;
}


void generateVolumes ( const std::string & volpath, VolumeList & vols )
{
    VolumeList::iterator vIter;

    std::string newpath;

    for ( vIter = vols.begin(); vIter != vols.end(); ++vIter )
    {
        Volume & vol = *vIter;
        newpath      = volpath;
        newpath.append("/").append(vol.name);
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
        for ( iIter = vIter->items.begin(); iIter != vIter->items.end(); ++iIter ) 
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


void usage()
{
    std::cout << "Usage: volgen  [-a:dDhLv:V]... <directory>" << std::endl
              << "   -a <dir> : Set archive directory. (default is " 
              << VOLGEN_ARCHIVEDIR << ")." << std::endl
              << "   -d       : Enable debug output and file statistics." << std::endl
              << "   -D       : Detailed volume layout. Default is a brief list." << std::endl 
              << "   -L       : List volume layout only, do not generate links." << std::endl
              << "   -v <mb>  : Set volume size in Mb (default is " 
              << VOLGEN_VOLUME_MB << ")." << std::endl
              << "   -h       : Display usage info and exit." << std::endl
              << "   -V       : Display version info and exit." << std::endl
              << std::endl;
    exit(0);
}


void version()
{
    std::cout << "volgen " << VOLGEN_VERSION << std::endl
        << VOLGEN_LICENSE << std::endl;
    exit(0);
}


} // namespace

using namespace volgen;



int main ( int argc, char **argv )
{
    std::string  curdir, target, voldir;
    char         optChar;
    char *       dirstr = NULL;
    long         volsz  = VOLGEN_VOLUME_MB;
    bool         debug  = false;
    bool         dogen  = true;
    bool         show   = false;

    while ( (optChar = getopt(argc, argv, "a:dDhLv:V")) != EOF ) 
    {
        switch ( optChar ) 
        {
            case 'a':
                dirstr = strdup(optarg);
                break;
            case 'd':
                debug = true;
                show  = true;
                break;
            case 'D':
                show  = true;
                break;
            case 'h':
                usage();
                break;
            case 'L':
                dogen = false;
                break;
            case 'v':
                volsz = ::atoi(optarg);
                break;
            case 'V':
                version();
                break;
        }
    }

    if ( argc < 2 )
        usage();
    if ( optind == argc ) {
        std::cout << "no target defined" << std::endl;
        usage();
    }

    target  = argv[optind];
    int cd  = ::chdir(target.c_str());

    if ( cd < 0 ) 
    {
        if ( errno == EACCES ) {
            std::cout << "No permission for " << target << std::endl;
            return -1;
        } else {
            std::cout << "Error with target: " << target << ": " 
                << std::string(strerror(errno)) << std::endl;
            return -1;
        }
    }

    if ( dirstr != NULL ) {
        voldir = dirstr;
        ::free(dirstr);
    } else {
        voldir = VOLGEN_ARCHIVEDIR;
    }

    DirTree    tree;
    VolumeList vols;
    
    curdir = volgen::getCurrentPath();
    
    if ( ! StringUtils::startsWith(voldir, "/") ) {
        std::string tmp = voldir;
        voldir = curdir;
        if ( ! StringUtils::endsWith(voldir, "/") )
            voldir.append("/");
        voldir.append(tmp);

        std::cout << "Archive dir set to " << voldir << std::endl;
    }

    if ( FileUtils::IsDirectory(voldir) && dogen ) {
        std::cout << "Error: directory already exists. Aborting..." << std::endl;
        return -1;
    } else if ( FileUtils::IsReadable(voldir) && dogen ) {
        std::cout << "Error: non-directory '" << voldir << "' already exists. Aborting..."
            << std::endl;
        return -1;
    }

    if ( ! volgen::readDirectory(curdir, tree, volsz, debug) ) {
        std::cout << "Fatal error reading directory" << std::endl;
        return -1;
    }

    if ( dogen && mkdir(voldir.c_str(), S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) < 0 ) {
        std::cout << "Error creating volgen archive dir '" << voldir << "' : " 
            << strerror(errno) << std::endl;
    }

    volgen::displayTree(curdir, target, tree);
    volgen::createVolumes(curdir, tree, vols);
    volgen::displayVolumes(vols, show);

    if ( dogen )
        volgen::generateVolumes(voldir, vols);
    else
        std::cout << "List only, no volumes generated." << std::endl;

    std::cout << "Finished." << std::endl;

    return 0;
}


//  _VOLGEN_MAIN_CPP_

