/**
  * @file volgen_main.cpp
  *
  * An application for generating volumes of a given size based
  * on a file system directory.  Intended for performing backups
  * to various media types such as DVD-RW, USB Flash, etc.
  *
  * Copyright (c) 2009-2023 Timothy C. Arland <tcarland@gmail.com>
  *
  * VolGen is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * VolGen is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with VolGen.  If not, see <https://www.gnu.org/licenses/>.
  *
 **/
#define _VOLGEN_MAIN_CPP_

#include <cstdlib>
#include <iostream>
#include <getopt.h>

#include "VolGen.h"
using namespace volgen;

#include "util/FileUtils.h"
#include "util/StringUtils.h"
using namespace tcanetpp;


void usage()
{
    std::cout << "Usage: volgen  [-a:dDhLs:V]... <directory>" << std::endl
        << "  -a | --archive <dir> : Set volgen meta directory; default is " << VOLGEN_ARCHIVEDIR << "." << std::endl
        << "  -d | --debug         : Enable debug output and file statistics." << std::endl
        << "  -h | --help          : Display usage info and exit." << std::endl
        << "  -D | --detail        : Detailed volume layout. Default is a brief list." << std::endl
        << "  -L | --list          : List volume layout only, do not generate metalinks." << std::endl
        << "  -s | --size  <mb>    : Set volume size in Mb (default is " << VOLGEN_VOLUME_MB << ")." << std::endl
        << "  -V | --version       : Display version info and exit." << std::endl
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

    static struct option l_opts[] = { {"archive", required_argument, 0, 'a'},
                                      {"debug",   no_argument, 0, 'd'},
                                      {"help",    no_argument, 0, 'h'},
                                      {"detail",  no_argument, 0, 'D'}, 
                                      {"list",    no_argument, 0, 'L'}, 
                                      {"size", required_argument, 0, 's'},
                                      {"version", no_argument, 0, 'V'}
                                    };
    int optindx = 0;

    while ( (optChar = ::getopt_long(argc, argv, "a:dDhLs:V", l_opts, &optindx)) != EOF )
    {
        switch ( optChar ) {
            case 'a':
                dirstr = ::strdup(optarg);
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
            case 's':
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
        std::cout << "volgen: No target defined" << std::endl;
        usage();
    }

    target  = argv[optind];
    int cd  = ::chdir(target.c_str());

    if ( cd < 0 )
    {
        if ( errno == EACCES ) {
            std::cout << "volgen: No permission for " << target << std::endl;
            return -1;
        } else {
            std::cout << "volgen: Error with target: " << target << ": "
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

        std::cout << "volgen: Archive dir set to " << voldir << std::endl;
    }

    if ( FileUtils::IsDirectory(voldir) && dogen ) {
        std::cout << "volgen Error: directory already exists. Aborting..." << std::endl;
        return -1;
    } else if ( FileUtils::IsReadable(voldir) && dogen ) {
        std::cout << "volgen Error: non-directory '" << voldir << "' already exists. Aborting..."
            << std::endl;
        return -1;
    }

    VolGen  vgen(curdir);

    vgen.setVolumeSize(volsz);
    vgen.setDebug(debug);

    if ( ! vgen.read() ) {
        std::cout << "volgen: Fatal error reading directory" << std::endl;
        return -1;
    }

    if ( dogen && ::mkdir(voldir.c_str(), S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) < 0 ) {
        std::cout << "volgen: Error creating volgen archive dir '" << voldir << "' : "
            << strerror(errno) << std::endl;
    }

    vgen.displayTree();
    vgen.createVolumes();
    vgen.displayVolumes(show);

    if ( dogen )
        vgen.generateVolumes(voldir);
    else
        std::cout << "volgen: List only, no volumes generated." << std::endl;

    std::cout << "volgen finished." << std::endl;

    return 0;
}
