#include <pspaudiorouting.h>
#include <stdio.h>
#include "main.h"

struct Options options={0,2.4f,10,1};

void loadOptions()
{
	options.invert=0;
	options.music=7;
	options.sfx=1;
	options.speed=0.4f;
	FILE *file=fopen("data/options","rb");
	if(file) {
		fread(&options,1,sizeof(options),file);
		fclose(file);
	}
	if(options.music<0 || options.music>10) options.music=7;
	if(options.invert!=0) options.invert=1;
	if(options.sfx!=0) options.sfx=1;
	if(options.speed<0.3f || options.speed>0.6f) options.speed=0.4f;
	songVolume(options.music);
}

