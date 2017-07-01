Disk Volume Generator or VolGen
===============================

  An application for generating volumes of a given size based on a file system directory.
Intended for performing backups to various media types such as DVD-RW, CD-RW, USB Flash, etc.

  Requires:
   * The 'tcanetpp' library:  https://github.com/tcarland/tcanetpp.git 
   * tcamake Makefile environment: https://github.com/tcarland/tcamake.git


 The tool builds a metadata type tree of volumes in '.volgen' of the current directory 
(by default) that represent the splitting of the current directory into no more than N 
bytes in size. The ***.volgen*** volumes contain soft links to the files that make up 
the given volume. Since .volgen contains primarily just links, it is relatively cheap 
to keep around or recreate. Note, that the difference between the current state versus 
when volgen was last ran can be determined using the voldiff script. This can be used 
to create a diff volume of just changes to keep archival media current.



