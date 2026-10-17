/*
 * Audio.cpp
 *
 *  Created on: 12/06/2013
 *      Author: Victor
 */

#include "Audio.h"

int Audio::musicVolume = MIX_MAX_VOLUME;
int Audio::soundVolume = MIX_MAX_VOLUME;
bool Audio::muted = false;
bool Audio::captionActive = false;

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

	// Asset ausente/corrompido: mensagem clara em vez de NULL por ai.
	if (sound == NULL && music == NULL)
		cerr << "Audio: nao foi possivel carregar '" << fileName << "': "
		     << Mix_GetError() << endl;
}

Audio::~Audio()
{
	if(sound)
	{
		stop();
		Mix_FreeChunk(sound);
	}
	if(music)
	{
		stop();
		Mix_FreeMusic(music);
	}
}

void Audio::play(int n)
{
	if(sound)
	{
		channel = Mix_PlayChannel(channel, sound, n);
		if (channel < 0)
			channel = Mix_PlayChannel(-1, sound, n);
	}
	else if(music)
		Mix_PlayMusic(music, n);
}

void Audio::stop()
{
	if(sound)
	{
		if (channel >= 0)
			Mix_HaltChannel(channel);
		channel = -1;
	}
	else if(music)
		Mix_HaltMusic();
}

void Audio::setVolume(int volume)
{
	if (volume < 0)
		volume = 0;
	if (volume > MIX_MAX_VOLUME)
		volume = MIX_MAX_VOLUME;

	if (sound)
		Mix_VolumeChunk(sound, volume);
	else if (music)
		Mix_VolumeMusic(volume);
}

int Audio::getMusicVolume()
{
	return musicVolume;
}

int Audio::getSoundVolume()
{
	return soundVolume;
}

bool Audio::isMuted()
{
	return muted;
}

bool Audio::isCaptionVolumeActive()
{
	return captionActive;
}

void Audio::setMusicVolume(int volume)
{
	if (volume < 0)
		volume = 0;
	if (volume > MIX_MAX_VOLUME)
		volume = MIX_MAX_VOLUME;

	musicVolume = volume;
	if (muted && volume > 0)
		muted = false;

	Mix_VolumeMusic(muted ? 0 : musicVolume);
	captionActive = true;
	refreshCaption();
}

void Audio::setSoundVolume(int volume)
{
	if (volume < 0)
		volume = 0;
	if (volume > MIX_MAX_VOLUME)
		volume = MIX_MAX_VOLUME;

	soundVolume = volume;
	if (muted && volume > 0)
		muted = false;

	Mix_Volume(-1, muted ? 0 : soundVolume);
}

void Audio::toggleMute()
{
	muted = !muted;
	Mix_VolumeMusic(muted ? 0 : musicVolume);
	Mix_Volume(-1, muted ? 0 : soundVolume);
	captionActive = true;
	refreshCaption();
}

void Audio::refreshCaption()
{
	string caption;

	if (muted)
		caption = string(GAME_NAME) + " | Musica: MUDO (M)";
	else
		caption = string(GAME_NAME) + " | Musica: " + std::to_string(musicVolume * 100 / MIX_MAX_VOLUME) + "% (+/-)";

	SDL_WM_SetCaption(caption.c_str(), caption.c_str());
}
