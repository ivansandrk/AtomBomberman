#include <errno.h>

#include "SDL.h"
#include "SDL_mixer.h"
#include "SDL_rwops_zzip.h"

#include "config.h"
#include "sound.h"


// sound_list and enums SOUND_* have to be in the same order
static const char *sound_list[] = {"1006.ogg", "1017.ogg", "1028.ogg", "1036.ogg", "1041.ogg", "1045.ogg", "1049.ogg", "1055.ogg", "1059.ogg", "1062.ogg", "1074.ogg", "2MUCPAIN.ogg", "ALARM10.ogg", "ALARM11.ogg", "ALARM12.ogg", "ALARM13.ogg", "ALARM1.ogg", "ALARM2.ogg", "ALARM3.ogg", "ALARM4.ogg", "ALARM5.ogg", "ALARM6.ogg", "ALARM7.ogg", "ALARM8.ogg", "ALARM9.ogg", "ALLRITE.ogg", "ASSKILL.ogg", "BACKUP1.ogg", "BACKUP.ogg", "BEEP.ogg", "BEEPS.ogg", "BMBSTOP1.ogg", "BMBSTOP2.ogg", "BMBTHRW1.ogg", "BMBTHRW3.ogg", "BMBTHRW4.ogg", "BMBTHRW5.ogg", "BMDROP1.ogg", "BMDROP2.ogg", "BMDROP3.ogg", "BOING1.ogg", "BOING2.ogg", "BOMB_01.ogg", "BOMB_02.ogg", "BOMB_03.ogg", "BOMB_04B.ogg", "BOMB_04.ogg", "BOMB_05.ogg", "BOMB_06.ogg", "BOMB_07B.ogg", "BOMB_07.ogg", "BOMB_08.ogg", "BOMB_09.ogg", "BOMB_10.ogg", "BOMB10.ogg", "BOMB_11.ogg", "BOMB11.ogg", "BOMB_12B.ogg", "BOMB12B.ogg", "BOMB_12.ogg", "BOMB12.ogg", "BOMB_13.ogg", "BOMB13.ogg", "BOMB_14.ogg", "BOMB14.ogg", "BOMB_15B.ogg", "BOMB15B.ogg", "BOMB_15.ogg", "BOMB15.ogg", "BOMB_16.ogg", "BOMB16.ogg", "BOMB_17.ogg", "BOMB17.ogg", "BOMB_18.ogg", "BOMB18.ogg", "BOMB_19.ogg", "BOMB19.ogg", "BOMB1.ogg", "BOMB_20.ogg", "BOMB20.ogg", "BOMB_21B.ogg", "BOMB21B.ogg", "BOMB_21.ogg", "BOMB21.ogg", "BOMB_22.ogg", "BOMB22.ogg", "BOMB_23.ogg", "BOMB23.ogg", "BOMB_24B.ogg", "BOMB24B.ogg", "BOMB_24.ogg", "BOMB24.ogg", "BOMB2.ogg", "BOMB3.ogg", "BOMB4B.ogg", "BOMB4.ogg", "BOMB5.ogg", "BOMB6.ogg", "BOMB7B.ogg", "BOMB7.ogg", "BOMB8.ogg", "BOMB9.ogg", "BOMBBOUN.ogg", "BOMBHIT1.ogg", "BOMBHIT2.ogg", "BOMBHIT3.ogg", "BOMBHIT4.ogg", "BOMBSTOP.ogg", "BRAKHEAD.ogg", "BRINGON.ogg", "BURNEDUP.ogg", "BUSTASS.ogg", "CALLDAD.ogg", "CLEAR.ogg", "CLIKPLAT.ogg", "COMEGET.ogg", "COOLPOP.ogg", "CRIBROWN.ogg", "CUL8R.ogg", "DABOMB.ogg", "DESTU1.ogg", "DESTU2.ogg", "DIE1.ogg", "DIS2A.ogg", "DISEASE1.ogg", "DISEASE2.ogg", "DISEASE3.ogg", "DRABOM.ogg", "DRAW.ogg", "EATDUST.ogg", "ELAUGH1.ogg", "ENRT1.ogg", "ENRT2.ogg", "EWTAHH.ogg", "EXPL6.ogg", "EXPLO1.ogg", "EXPLODE2.ogg", "EXPLODE3.ogg", "EXPLODE4.ogg", "FEELPOWR.ogg", "FILLCRAK.ogg", "FIREINH.ogg", "GEN8A.ogg", "GET1.ogg", "GET2.ogg", "GODHERE.ogg", "GOTAHURT.ogg", "GOTCHA.ogg", "GRAB1.ogg", "GRAB2.ogg", "GUMP1.ogg", "HAILGON.ogg", "HITSTRID.ogg", "HURRY.ogg", "HURYTUF.ogg", "INZONE.ogg", "JESUS.ogg", "JJFM1.ogg", "JJFM2.ogg", "JJFM3.ogg", "JJFM4A.ogg", "JJFM4B.ogg", "JJFM5.ogg", "JJFM8.ogg", "JMB10.ogg", "JMB11.ogg", "JMB12.ogg", "JMB14.ogg", "JMB15.ogg", "JMB16.ogg", "JMB1.ogg", "JMB2.ogg", "JMB3.ogg", "JMB4.ogg", "JMB5.ogg", "JMB6.ogg", "JMB7.ogg", "JMB8.ogg", "JMB9.ogg", "JOFM1.ogg", "JOFM2.ogg", "KBOMB1.ogg", "KBOMB2.ogg", "KICKER10.ogg", "KICKER3.ogg", "KISSWOOK.ogg", "LATER.ogg", "LETTER1.ogg", "LETTER2.ogg", "LITEMUP.ogg", "LOOKOUT.ogg", "LOSE.ogg", "LOVNIT1.ogg", "LUVYAP.ogg", "MENUEXIT.ogg", "MENU.ogg", "MOLASES.ogg", "MYASS1.ogg", "NETWORK.ogg", "OHHHH.ogg", "OHNO1.ogg", "OHYEAH.ogg", "PACKIN.ogg", "PGAD12B.ogg", "PLAYPROP.ogg", "POOPS1.ogg", "POOPS2.ogg", "POOPS3.ogg", "POOPS4.ogg", "POOPS5.ogg", "PROUD.ogg", "QUITGAME.ogg", "ROASTED.ogg", "RUNAWAY1.ogg", "RUNAWAY2.ogg", "SCHWING.ogg", "SCREAM1.ogg", "SERENDIP.ogg", "SHAKEBOO.ogg", "SMELSMOK.ogg", "SQRDROP2.ogg", "SQRDROP4.ogg", "SQRDROP5.ogg", "SQRDROP6.ogg", "SQRDROP7.ogg", "SQRDROP8.ogg", "SQRDRPS2.ogg", "SQRDRPS3.ogg", "STUPIDIO.ogg", "SUCKITDN.ogg", "TASTPAI2.ogg", "TASTPAIN.ogg", "THEMAN.ogg", "TOEASY.ogg", "TRAMPO.ogg", "TRANSIN.ogg", "TRANSOUT.ogg", "WANTMNKY.ogg", "WARP1.ogg", "WHATRUSH.ogg", "WHODAD.ogg", "WIN.ogg", "WLKTAL1.ogg", "WLKTAL2.ogg", "WOOHOO1.ogg", "YEEHAW.ogg", "YOASSGRS.ogg", "YOUBLOW.ogg", "YOUWIN1.ogg", "ZEN1.ogg", "ZEN2.ogg"};

static Mix_Chunk *sound_chunk[SOUND_LAST];

// represents the channel a looping sound is played on
static int sound_loop_channel[SOUNDS_LAST];

static int *sounds_lookup[] = {
	(int[]) {2, SOUND_ENRT1, SOUND_ENRT2}, // SOUNDS_ERROR
	(int[]) {3, SOUND_BMDROP2, SOUND_BMDROP3, SOUND_BMDROP1}, // SOUNDS_BOMB_DROP
	(int[]) {4, SOUND_KBOMB1, SOUND_KBOMB2, SOUND_KICKER3, SOUND_KICKER10}, // SOUNDS_BOMB_KICK
	(int[]) {3, SOUND_BOMBSTOP, SOUND_BMBSTOP1, SOUND_BMBSTOP2}, // SOUNDS_BOMB_STOP
	(int[]) {3, SOUND_BOMBBOUN, SOUND_1017, SOUND_1036}, // SOUNDS_BOMB_JELLY_BOUNCE
	(int[]) {7, SOUND_CLIKPLAT, SOUND_SQRDROP2, SOUND_SQRDROP4, SOUND_SQRDROP5, SOUND_SQRDROP6, SOUND_SQRDROP7, SOUND_SQRDROP8}, // SOUNDS_TILE_SLAM
	(int[]) {2, SOUND_KBOMB1, SOUND_KBOMB2}, // SOUNDS_BOMB_PUNCH
	(int[]) {1, SOUND_BMDROP3}, // SOUNDS_BOMB_FLYING_BOUNCE
	(int[]) {6, SOUND_GRAB1, SOUND_GRAB2, SOUND_BMBTHRW1, SOUND_BMBTHRW3, SOUND_BMBTHRW4, SOUND_BMBTHRW5}, // SOUNDS_BOMB_GRAB
	(int[]) {20, SOUND_EXPL6, SOUND_EXPLO1, SOUND_EXPLODE2, SOUND_EXPLODE3, SOUND_EXPLODE4, SOUND_BOMB_01, SOUND_BOMB_02, SOUND_BOMB_04, SOUND_BOMB_04B, SOUND_BOMB_05, SOUND_BOMB_06, SOUND_BOMB_07, SOUND_BOMB_07B, SOUND_BOMB_09, SOUND_BOMB11, SOUND_BOMB12, SOUND_BOMB12B, SOUND_BOMB17, SOUND_BOMB19, SOUND_BOMB24}, // SOUNDS_BOMB_EXPLODE
	(int[]) {2, SOUND_SCREAM1, SOUND_DIE1}, // SOUNDS_DIE_FLAMES
	(int[]) {3, SOUND_PROUD, SOUND_THEMAN, SOUND_YOUWIN1}, // SOUNDS_WINNER
	(int[]) {1, SOUND_BURNEDUP}, // SOUNDS_DEATH_ANIM
	(int[]) {4, SOUND_1017, SOUND_1036, SOUND_1045, SOUND_TRAMPO}, // SOUNDS_STEP_TRAMPOLINE
	(int[]) {4, SOUND_BOMBHIT1, SOUND_BOMBHIT2, SOUND_BOMBHIT3, SOUND_BOMBHIT4}, // SOUNDS_STUNNED
	(int[]) {13, SOUND_WOOHOO1, SOUND_GET1, SOUND_GET2, SOUND_COOLPOP, SOUND_ALLRITE, SOUND_SCHWING, SOUND_1006, SOUND_1028, SOUND_1041, SOUND_1055, SOUND_1059, SOUND_1062, SOUND_1074}, // SOUNDS_GOOD_POWERUP
	(int[]) {5, SOUND_POOPS1, SOUND_POOPS2, SOUND_POOPS3, SOUND_POOPS4, SOUND_POOPS5}, // SOUNDS_POOP
	(int[]) {14, SOUND_CRIBROWN, SOUND_CUL8R, SOUND_GOTAHURT, SOUND_GOTCHA, SOUND_LATER, SOUND_ROASTED, SOUND_TOEASY, SOUND_YOUBLOW, SOUND_EATDUST, SOUND_SMELSMOK, SOUND_STUPIDIO, SOUND_SUCKITDN, SOUND_TASTPAIN, SOUND_TASTPAI2}, // SOUNDS_TAUNT
	(int[]) {6, SOUND_CLEAR, SOUND_FIREINH, SOUND_LOOKOUT, SOUND_LITEMUP, SOUND_RUNAWAY1, SOUND_RUNAWAY2}, // SOUNDS_STRING
	(int[]) {33, SOUND_BACKUP, SOUND_BACKUP1, SOUND_BRINGON, SOUND_BRINGON, SOUND_COMEGET, SOUND_DABOMB, SOUND_ELAUGH1, SOUND_FEELPOWR, SOUND_OHYEAH, SOUND_PACKIN, SOUND_THEMAN, SOUND_WHATRUSH, SOUND_WHODAD, SOUND_YEEHAW, SOUND_DESTU1, SOUND_DESTU2, SOUND_ZEN1, SOUND_ZEN2, SOUND_CALLDAD, SOUND_DRABOM, SOUND_FILLCRAK, SOUND_INZONE, SOUND_HITSTRID, SOUND_INZONE, SOUND_LOVNIT1, SOUND_LUVYAP, SOUND_PLAYPROP, SOUND_SERENDIP, SOUND_SHAKEBOO, SOUND_WANTMNKY, SOUND_WLKTAL1, SOUND_WLKTAL2, SOUND_WANTMNKY}, // SOUNDS_AWESOME
	(int[]) {2, SOUND_HURRY, SOUND_HURYTUF}, // SOUNDS_HURRY
};

int sound_init()
{
	int i;
	int flags = MIX_INIT_OGG;
	
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1)
	{
		fprintf(stderr, "Error initialising SDL Audio subsystem: %s\n", SDL_GetError());
		return -1;
	}
	
	if ((Mix_Init(flags)&flags) != flags)
	{
		fprintf(stderr, "Error initialising mixer: %s\n", Mix_GetError());
		return -1;
	}
	
	if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
	{
		fprintf(stderr, "Error opening audio: %s\n", Mix_GetError());
		return -1;
	}
	
	Mix_AllocateChannels(256);
	
	for (i = 0; i < SOUNDS_LAST; i++)
		sound_loop_channel[i] = -1;
	
	return 0;
}

int sound_quit()
{
	Mix_CloseAudio();
	
	while (Mix_Init(0))
	{
		Mix_Quit();
	}
	
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	
	return 0;
}

static Mix_Chunk* sound_load_chunk(const char *path)
{
	SDL_RWops *data;
	Mix_Chunk *chunk;
	
	data = SDL_RWFromZZIP(path, "r");
	if (!data)
	{
		fprintf(stderr, "Unable to open data [%s]: %s\n", path, strerror(errno));
		return 0;
	}
	
	chunk = Mix_LoadWAV_RW(data, 1);
	if (!chunk)
	{
		fprintf(stderr, "Unable to load audio file: %s\n", Mix_GetError());
		return 0;
	}
	
	return chunk;
}

static int sound_play_chunk(int sound, int loop)
{
	static char buf[1024];
	int ret, i, r;
	
	if (sound > SOUND_LAST)
	{
		i = sound - SOUND_LAST - 1;
		r = 1 + (rand() % sounds_lookup[i][0]);
		sound = sounds_lookup[i][r];
	}
	
	if (!sound_chunk[sound])
	{
		sprintf(buf, PATH_SOUND, sound_list[sound]);
		sound_chunk[sound] = sound_load_chunk(buf);
		
		if (!sound_chunk[sound])
		{
			return -1;
		}
	}
	
	ret = Mix_PlayChannel(-1, sound_chunk[sound], loop);
	
	if (ret == -1)
	{
		fprintf(stderr, "Error playing chunk [%s]: %s\n", sound_list[sound], Mix_GetError());
	}
	
	return ret;
}

int sound_play(int sound)
{
	if (sound_play_chunk(sound, 0) == -1)
	{
		return -1;
	}
	
	return 0;
}

int sound_play_loop(int sound)
{
	// sound is already playing (looping)
	if (sound_loop_channel[sound] != -1)
	{
		return 0;
	}
	
	sound_loop_channel[sound] = sound_play_chunk(sound, -1);
	if (sound_loop_channel[sound] == -1)
	{
		return -1;
	}
	
	return 0;
}

int sound_stop_loop(int sound)
{
	// sound isn't playing (looping)
	if (sound_loop_channel[sound] == -1)
	{
		return 0;
	}
	
	Mix_HaltChannel(sound_loop_channel[sound]);
	sound_loop_channel[sound] = -1;
	
	return 0;
}
