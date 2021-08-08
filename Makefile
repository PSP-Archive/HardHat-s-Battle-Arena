TARGET = battlearena
OBJS = main.o sound.o image.o gutools.o battlearena.o duel.o map.o particle.o md2fast.o wavefront.o options.o ase.o intrafont/libccc.o intrafont/intraFont.o terrain.o savejpeg.o team.o cutscene.o

CFLAGS = -g -O2 -G0 -Wall -Iintrafont -DMIKMOD_PLAYER=1 
#CFLAGS = -g -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LIBS = -llua -llualib -lcurl -lpsprtc -lpspgum_vfpu -lpspvfpu -lpspgu -lpng -lz -lm -lmikmod -lmmio -lpspaudiolib -lpspaudio -lpsppower -lpsphttp -lpspopenpsid -ljpeg
# -lvorbisfile -lvorbisenc -lvorbis -logg
LDFLAGS = -g

PSP_FW_VERSION=200
BUILD_PRX=1

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Sushi Battle Arena
PSP_EBOOT_ICON = icon0.png
PSP_EBOOT_PIC1 = pic1.png
#PSP_EBOOT_SND0 = snd0.at3

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

battlearena.c: iconpic.h

iconpic.h: icon0.png pic1.png
	bin2c icon0.png icon0_data icon0
	bin2c pic1.png pic1_data pic1
	cat icon0_data pic1_data >iconpic.h
	rm icon0_data pic1_data

dist: EBOOT.PBP readme.txt Makefile
	-rm -rf dist
	mkdir dist
	mkdir dist/$(TARGET)
	mkdir dist/src
	mkdir dist/src/intrafont
	cp -p *.[ch] Makefile* icon*.png dist/src/.
	cp -p intrafont/*.[ch] dist/src/intrafont/.
	-cp -p pic*.png dist/src/.
	-cp -p snd*.at3 dist/src/.
	cp -p readme.txt license.txt dist/.
	cp -p EBOOT.PBP dist/$(TARGET)
	mkdir dist/$(TARGET)/data
	mkdir dist/$(TARGET)/models
	-cp editor/devcpp/dist/*.dll editor/devcpp/dist/*.exe dist/editor/.
	-cp -p data/* dist/$(TARGET)/data
	-cp -p -r models/* dist/$(TARGET)/models
	-rm -r `find dist/$(TARGET)/models -type d|grep svn`
	-rm dist/$(TARGET)/data/hiscore.txt
	-rm dist/$(TARGET)/data/options
	-rm $(TARGET)dist.zip 
	(cd dist; zip -r ../$(TARGET)dist.zip .)


