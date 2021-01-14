VolGen - Disk Volume Generator
===============================

  A tool for creating disk *volumes* from a given filesystem path.
This is intended for performing backups to static media types such 
as DVD-RW, CD-RW, USB Flash, or for manageable large objects in cold 
storage.

  Requires:
   * The *tcamake* gnumake wrapper: https://github.com/tcarland/tcamake.git
   * The *tcanetpp* library:  https://github.com/tcarland/tcanetpp.git

The tool builds a tree of volumes in the special path `.volgen` of the  
working directory (by default, but can be defined by providing `--archive`)
that represents the splitting of the target directory into volumes of 
no more than 'n' bytes in size. 

The volume directories contain soft-links to the files that make up the 
given volume. Since *.volgen* contains just links, it is cheap to keep 
around or recreate. These volumes can then be archived using the 
*follow-links* option of the corresponding archival tool.

The difference between the current *volgen* state and when *volgen* was 
last ran can be determined using the *voldiff* script.  This, in turn, 
can be used to create *diff* volumes to keep the archival/backup media 
current while avoiding a rewrite of all volumes.

## Building VolGen

- Obtain the requirements.
  ```
  $ mkdir src && cd src
  $ git checkout https://github.com/tcarland/tcamake.git
  $ git checkout https://github.com/tcarland/tcanetpp.git
  $ git checkout https://github.com/tcarland/volgen.git
  ```

- Build the tcanetpp library. 
  ```
  $ cd tcanetpp 
  $ source resources/tcanetpp_release_mt
  $ make
  ```

- Ensure the taglib library is installed, including the 
  *development* package.
  ```
  $ sudo apt install libtag1-dev   # ubuntu
  # or 
  $ sudo yum install taglib-devel  # rhel
  ```

- Build VolGen
  ```
  $ cd ../volgen
  $ export TCAMAKE_PROJECT=1
  $ make
  ```