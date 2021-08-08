#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <psptypes.h>
#include <psprtc.h>

#ifdef MIKMOD_PLAYER
#include <mikmod.h>
#endif
#define MUS_INTRO 0
#define MAX_MUSIC 4

#include "main.h"

#ifdef MIKMOD_PLAYER
SAMPLE *sfx[MAX_SFX];
SAMPLE *wav;
UNIMOD *song[MAX_MUSIC];
#endif
int currentSong;

char *sfxfilename[]={	"data/menunext.wav","data/menuselect.wav",
	"data/target.wav","data/pain.wav","data/chaos.wav",
	"data/explode.wav","data/powerup.wav","data/win.wav",
	"data/lose.wav","data/bossdamage.wav",NULL };
char *musicfilename[]={ "data/vibe-suw.it", 
	"data/daydream.xm", 
	"data/vibe-suw.it",
	"data/daydream.xm" };
u64 songStart=0;

#ifdef MIKMOD_PLAYER
void mikmodErrorHandler(void)
{
        printf("_mm_critical %d\n", _mm_critical);
        printf("_mm_errno %d\n", _mm_errno);
        printf("%s\n", _mm_errmsg[_mm_errno]);
        return;
}
#endif

void initSound()
{
#ifdef MIKMOD_PLAYER
    _mm_RegisterErrorHandler(mikmodErrorHandler);
    MikMod_RegisterAllLoaders();
    MikMod_RegisterAllDrivers();
    md_mode = DMODE_16BITS|DMODE_STEREO|DMODE_SOFT_SNDFX|DMODE_SOFT_MUSIC;
    md_reverb = 0;
    md_pansep = 128;
    MikMod_Init();
    MikMod_SetNumVoices(-1, 8);

    FILE *file=fopen(musicfilename[MUS_INTRO],"rb");
    if(file) {
	fclose(file);
	song[MUS_INTRO]=MikMod_LoadSong(musicfilename[MUS_INTRO],128);
	sceRtcGetCurrentTick(&songStart);
	Player_Start(song[MUS_INTRO]);
    } else {
	printf("couldn't find '%s'\n",musicfilename[MUS_INTRO]);
    }

    int i;
    for(i=0;i<MAX_SFX;i++) {
        printf("reading sound file %d...\n",i);
        FILE *file=fopen(sfxfilename[i],"rb");
        if(file) {
            //printf("opening file %s\n",sfxfilename[i]);
            fclose(file);
            sfx[i]=WAV_LoadFN(sfxfilename[i]);
        }
        if(!sfx[i]) printf("Warning: missing sound file '%s'\n",sfxfilename[i]);
    }
#endif
}

void updateSound()
{
	u64 newTick;
	
	sceRtcGetCurrentTick(&newTick);

#ifdef MIKMOD_PLAYER
	if(song[currentSong]==0) return;
    if(MikMod_Active()==0) {
		Player_Stop();
		MikMod_FreeSong(song[currentSong]);
		
		song[currentSong]=MikMod_LoadSong(musicfilename[currentSong],128);
		Player_Start(song[currentSong]);
		sceRtcGetCurrentTick(&songStart);
	}
#endif
}

void playSfx(enum Sfx id)
{
	if(options.sfx==0) return;
#ifdef MIKMOD_PLAYER
	if(id>=0 && id<MAX_SFX && sfx[id]!=0) {
		/* int voice = */ MikMod_PlaySample(sfx[id],0,0);
		//printf("playing %s on voice %d\n",sfxfilename[id],voice);
	} else {
		if(id>=0 && id<MAX_SFX)
			printf("Couldn't play %d sample (%s).  Couldn't find it.\n",id,sfxfilename[id]);
		else
			printf("Couldn't play %d sample.  Couldn't find it.\n",id);
	}
#ifdef SFX_OPENING
    if(id==SFX_OPENING) {
		Player_Stop();
		MikMod_FreeSong(song[MUS_INTRO]);
		
		song[MUS_INTRO]=MikMod_LoadSong(musicfilename[MUS_INTRO],128);
		Player_Start(song[MUS_INTRO]);
		sceRtcGetCurrentTick(&songStart);
	}
#endif
#endif
}

void playSong(int id)
{
	if(currentSong==id) return;
#ifdef MIKMOD_PLAYER
	Player_Stop();
	if(song[currentSong]) MikMod_FreeSong(song[currentSong]);
	song[currentSong]=0;

    FILE *file=fopen(musicfilename[MUS_INTRO],"rb");
    if(file) {
		fclose(file);
		currentSong=id;
		song[currentSong]=MikMod_LoadSong(musicfilename[currentSong],128);
		Player_Start(song[currentSong]);
		sceRtcGetCurrentTick(&songStart);
    } else {
		printf("couldn't find '%s'\n",musicfilename[MUS_INTRO]);
    }
	
#endif
}

void playSound(int id)
{
        //if(game.muteSfx) return;
#ifdef MIKMOD_PLAYER
        if(id>=0 && id<MAX_SFX && sfx[id]!=0) {
                int voice = MikMod_PlaySample(sfx[id],0,0);
                printf("playing %s on voice %d\n",sfxfilename[id],voice);
        }
#endif
        return;
}

#ifdef MIKMOD_PLAYER
SAMPLE *wav=0;
#endif

void playWav(const char *fname)
{
#ifdef MIKMOD_PLAYER
	if(wav) {
		WAV_Free(wav);
		wav=0;
	}
    FILE *file=fopen(fname,"rb");
    if(file) {
        //printf("opening file %s\n",sfxfilename[i]);
        fclose(file);
		wav=WAV_LoadFN((char *)fname);
    }
	if(!wav) {
		char buf[256];
		strcpy(buf,"map/song/");
		strcat(buf,fname);
	    FILE *file=fopen(buf,"rb");
	    if(file) {
	        //printf("opening file %s\n",sfxfilename[i]);
	        fclose(file);
			wav=WAV_LoadFN(buf);
	    }
	}
	if(!wav) return;
	int voice = MikMod_PlaySample(wav,0,0);
	printf("playing %s on voice %d\n",fname,voice);
#endif
}

void freeWavs()
{
#ifdef MIKMOD_PLAYER
	if(wav)	WAV_Free(wav);
	wav=0;
#endif
}

void songVolume(int vol)
{
#ifdef MIKMOD_PLAYER
	Player_SetVolume(128*options.music/10);
#endif
}
