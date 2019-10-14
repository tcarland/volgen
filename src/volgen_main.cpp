/**
  *  An application for generating volumes of a given size based
  *  on a file system directory.
  *  Intended for performing backups to various media types
  *  such as DVD-RW, CD-RW, USB Flash, etc.
  *
  * @file   volgen_main.cpp
  * @author tcarland@gmail.com
  *
  * Copyright (c) 2009-2012 Timothy C. Arland <tcarland@gmail.com>
  *
  *
 **/
#define _VOLGEN_MAIN_CPP_

#include <cstdlib>
#include <iostream>


#include "VolGen.h"
using namespace volgen;

#include "util/FileUtils.h"
using namespace tcanetpp;


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

    curdir = VolGen::GetCurrentPath();

    if ( ! StringUtils::StartsWith(voldir, "/") ) {
        std::string tmp = voldir;
        voldir = curdir;
        if ( ! StringUtils::EndsWith(voldir, "/") )
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

    VolGen  vgen(curdir);

    vgen.setVolumeSize(volsz);
    vgen.setDebug(debug);

    if ( ! vgen.read() ) {
        std::cout << "Fatal error reading directory" << std::endl;
        return -1;
    }

    if ( dogen && ::mkdir(voldir.c_str(), S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) < 0 ) {
        std::cout << "Error creating volgen archive dir '" << voldir << "' : "
            << strerror(errno) << std::endl;
    }

    vgen.displayTree();
    vgen.createVolumes();
    vgen.displayVolumes(show);

    if ( dogen )
        vgen.generateVolumes(voldir);
    else
        std::cout << "List only, no volumes generated." << std::endl;

    std::cout << "Finished." << std::endl;

    return 0;
}
