Disk Volume Generator or VolGen
===============================

  An application for generating volumes of a given size based on a file system directory.
Intended for performing backups to various media types such as DVD-RW, CD-RW, USB Flash, etc.

  Requires the 'tcanetpp' library and tcamake to build.


 The app essentially builds a metadata type tree of volumes in '.volgen' of the current directory (by default). The .volgen volumes contain soft links to the files that make up the volume. Since .volgen would contain primarily just links, it is relatively cheap to keep around or recreate. Note, that the difference between the current state versus when volgen was last rand can be determined using the voldiff script.



