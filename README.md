VolGen - Disk Volume Generator
===============================

A tool for creating disk *volumes* from a given filesystem path.
This is intended for performing backups to static media types such 
as DVD-RW, CD-RW, USB Flash, or for other, more manageable large 
objects in cold storage. 

<notice>
* VolGen - Volume Generator
*
* Copyright (c) 2009-2025 Timothy C. Arland <tcarland@gmail.com>
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
</notice>

## Overview

Requirements:
 * The [tcamake](https://github.com/tcarland/tcamake.git) gnu-make wrapper
 * The [tcanetpp](https://github.com/tcarland/tcanetpp.git) library

**VolGen** operates by creating a metadata directory, `.volgen`, in 
the working directory (or as specified), made up of directories of 
softlinks that each represent the *sized* volume. These directories 
essentially *split* the target directory by the configured 'n' bytes 
in size.

The volume directories contain soft-links to the files that make up the 
given volume. Since *.volgen* contains only links, it is cheap to keep 
around or recreate. These volumes can then be archived using the 
*follow-links* option of any corresponding archival tool (eg. rsync).

The difference between the current *volgen* state and when *volgen* was 
last ran can be determined using the *voldiff* script.  This, in turn, 
can be used to create *diff* volumes to keep the archival/backup media 
current while avoiding a rewrite of all volumes.

## Building VolGen

- Obtain the requirements.
  ```bash
  mkdir src && cd src
  git checkout https://github.com/tcarland/tcamake.git
  git checkout https://github.com/tcarland/tcanetpp.git
  git checkout https://github.com/tcarland/volgen.git
  ```

- Build the tcanetpp library. The build environment for the 
  lib is needed and can be found or defined in *resources*.
  ```bash
  cd tcanetpp 
  source resources/tcanetpp_release_mt
  make arlib
  ```

- Build VolGen
  ```bash
  cd ../volgen
  make
  ```
