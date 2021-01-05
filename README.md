VolGen - Disk Volume Generator
===============================

  A tool for creating disk *volumes* from a given filesystem path.
This is intended for performing backups to static media types such 
as DVD-RW, CD-RW, USB Flash, or for manageable large objects in cold 
storage.

  Requires:
   * The *tcamake* gnumake wrapper: https://github.com/tcarland/tcamake.git
   * The *tcanetpp* library:  https://github.com/tcarland/tcanetpp.git

  The tool builds a tree of volumes in the path '.volgen' of the current 
working directory (by default) that represents the splitting of the target 
directory into volumes of no more than 'n' bytes in size. The volume 
directories contain soft-links to the files that make up the given volume. 
Since *.volgen* contains just links, it is cheap to keep around or recreate. 
These volumes can then be archived using the 'follow-links' option of the 
requisite archival tool (rsync, tar, mkiso, etc).

The difference between the current *volgen* state and when *volgen* was last
ran can be determined using the voldiff script.  This, in turn, can be used 
to create *diff* volumes to keep the archival/backup media current while 
avoiding a rewrite of all volumes.
