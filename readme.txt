HardHat's Battle Arena
By Team Sushi http://www.teamsushi.org/

Welcome to Sushi Island.  As an aspiring Mage Master, your mission is to 
collect and train a party of up to 6 low level mages and eventually win the
Sushi Island Tournament.

You start with one of 29 Mage Apprentices and through battles of skill and 
strength, you can upgrade their experience points, gradually growing 
stronger and stronger mages until you are ready to enter the tournament
for the top prize of the Sushi Cup.

Installation:
Required features of the 2.0 or later Custom Firmwares.  It will appear to 
run from 1.5 or from CFW's minimal 1.5, but will not work.

Install in ms0:/psp/game or if that is set for 1.5 compatibility put it in
the corresponding game4xx type folder.

Controls:
d-pad to walk around the island, X to choose a battle, O to save progress
d-pad plus X to select attacks

Credits:
Programming: HardHat
Environment models: Meyitzo, DimensionT, loopix-project.com
Textures: fatcat
Character models: http://www.alternatedimension.com/models.html, see readmes in included models
Music: http://modarchive.org
Sounds:  http://www.meanrabbit.com
Cut scene story: wicked.fable
Beta Testers: jj_calvin, wicked.fable, and #psp-programming of course
Includes: intraFont by Ben Hur, libpng, libjpeg, zlib, mikmod and of course the ps2dev.org SDK.  Thanks for the tools.


Change log:
Version 0.61:
- added trainers who guide you through the whole Mage Master experience.
- fixed underpowered spells from increasing your opponent's health.  It now says attack was ineffective.
- moved game save to START so that those O saving loops go away
- use O to exit from battle preparation screens.
- all new mage upgrades now happen at green or white training centres
- rearranged the order of the training centres to have a more natural flow around the road, and switched the game save format.  The order is now: white, yellow, red, blue, black (tournament) and green.
Version 0.52:
- made cheat repair for accidental cheat mode activation with the START button.
- made the cheat mode only work if you specifically add a "cheats" file.
Version 0.51:
- changed memory management strategy to involved less churn
- removed duplicated model, by using shared animmesh
- changed allocation/deallocation to reduce memory fragmentation
- fixed minor typos in the last 4 cut scenes
- fixed massive memory leak in Wavefront OBJ loader
- added cheat code to speed testing
Version 0.50:
- fixed texture on opening sequence to GU_REPEAT instead of GU_CLAMP
- animated chest/crate smashing
- show item in chest/crate
- save game feature disabled if you are in the 1.5 folder.
- fixed getting caught on the chest when it opens
- added in many cut scenes
- new memory leak detected.  Sorry.  Save often, I guess until I can fix.
Version 0.40:
- added basic collision detection on the island
- fixed teleport so you never wind up partly inside the building
- tuned the look of the brick house
- rescaled the trees to be a better size, and accomodate collision detection
- roughed in the rest of the cut scenes (should be in the next build)
- roughed in the items menu (should work in the next build hopefully)
- tuned the miss symantics to be fairer
Version 0.30:
- Fixed crash bug on map screen with too many messages.  Now overrides the currently displayed message
- Really fixed the "attacks sometimes do nothing, and no message" problem.
- Turned on screen clearing, since it bothered people to have it off when they 
did camera tricks on me.
- Totally revamped the save/load system.  Now has inventory saved correctly. 
Now loads all party members correctly.
- Fixed the trophy screen when you win tournamnets at the black training 
centres.
- Added 9 new spell upgrades to level 61
- Now shows the crate count and chest count on screen properly
- Percent complete is now fixed, which should fix the abort save exiting the 
game
- Tuned the particle effects when walking around the island

Version 0.20:
- Better cut scene player with fixed throne room polygons.  Thanks jsharrad
- Fixed: hitting X repeatedly skips doDamage!
- Can now smash crates and chests, and collect what is inside
- There are now 29 spells that your party of mages earn over time, with a scroll
ing list
- Each of the six training centres has a distinctive battle style
- Change battle terrain based on place we go to
- Game save format update
- Moved some crates and chests so that they are in better positions
- Teleport once again works as designed, and you are less likely to wind up in a
 building.
- Near object detection is fixed again
- Tuned the default difficulty of the battles to be much more interesting
- Now highlights which training centre you should visit next.
Still more to come
Version 0.11:
- initial release

