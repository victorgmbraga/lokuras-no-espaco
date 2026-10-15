/*
 * Audio.cpp
 *
 *  Created on: 12/06/2013
 *      Author: Victor
 */

#include "Audio.h"

Audio::Audio(string fileName, int type)
{
	this->fileName = fileName;
	channel = -1;

	if(type == AUDIOTYPE_SOUND)
	{
		sound = Mix_LoadWAV(fileName.c_str());
		music = NULL;
	}
	else if(type == AUDIOTYPE_MUSIC)
	{
		music = Mix_LoadMUS(fileName.c_str());
		sound = NULL;
	}
}

Audio::~Audio()
{
	if(sound)
			Mix_FreeChunk(sound);
	if(music)
		Mix_FreeMusic(music);
}

void Audio::play(int n)
{
	if(sound)
		channel = Mix_PlayChannel(channel, sound, n);
	else if(music)
		Mix_PlayMusic(music, n);
}

void Audio::stop()
{
	if(sound)
		Mix_HaltChannel(channel);
	else if(music)
		Mix_FadeOutMusic(0);
}
