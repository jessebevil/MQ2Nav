MQ2Nav Changelog
================


2.0.1
-----
*NOTE* This version is not compatible with navmeshes made with previous versions. You will need to rebuild your meshes!

* Various visual improvements to UI.
* Revamp the Waypoints UI, fixing several bugs
  * Added option to delete and rename waypoints
  * Fixed the order of the x,y,z coordinates in waypoints UI to match /loc
* Tile size limit has been removed. Tiles are now indexed by 64-bit integer so the tile limit is so high now that you'll run out of memory before you hit it.
* Due to many changes to the internal structure of the navmesh, old navmeshes will not load correctly. As a result, the navmesh compat version has been bumped up, so loading older meshes will give you an error.
* Adjusted the defaults of a couple navmesh parameters for better path finding behavior:
  * Reduced MaxClimb to 4.0 to avoid getting stuck on the edge of tall objects
  * Increased MaxSlope to 75 to allow traversal over steeper terrain.
* Fixed various /nav commands and made many improvements:
  * Fixed /nav x y z
  * Added /nav spawn <text> - takes same parameters of Spawn TLO to do a spawn search
  * Fixed issues with clicking items and doors when using /nav item or /nav door
  * Added new forms of /nav commands, see the wiki: https://github.com/brainiac/MQ2Nav/wiki/Command-Reference
* Remove spam click behavior of /nav item/door as it didn't work right anyways
* Improvements to how the active navigation path line is rendered.


2.0.0
-----

Initial Version of change log, started recording changes after this point!