Disk Volume Generator or VolGen
===============================

  An application for generating volumes of a given size based on a file system directory.
Intended for performing backups to various media types such as DVD-RW, CD-RW, USB Flash, etc.

  Requires the 'tcanetpp' library and tcamake to build.


 The app essentially builds a metadata type tree of the volumes in '.volgen' directory (by default). This contains soft links to the files that make up the volume. Since .volgen would contain primarily just links, it is cheap to keep around. You can see the difference between the current volumes since last run by using the voldiff tool.


