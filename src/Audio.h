/*
 * Audio.h
 *
 *  Created on: 12/06/2013
 *      Author: Victor
 */

#ifndef AUDIO_H_
#define AUDIO_H_

#define AUDIOTYPE_SOUND 0
#define AUDIOTYPE_MUSIC 1

#include "SDLBase.h"

class Audio
{
	private:
		string fileName;
		Mix_Chunk *sound;
		Mix_Music *music;
		int channel;

		// Estado global de volume (compartilhado por todas as instancias e
		// usado pelas teclas de volume do StateManager).
		static int musicVolume;   // 0..MIX_MAX_VOLUME
		static int soundVolume;   // 0..MIX_MAX_VOLUME
		static bool muted;
		static bool captionActive; // o titulo da janela mostra o volume?
	public:
		Audio(string fileName, int type);
		virtual ~Audio();
		void play(int n);
		void stop();
		// Volume desta instancia (0..128): Mix_VolumeChunk ou Mix_VolumeMusic.
		void setVolume(int volume);

		static int getMusicVolume();
		static int getSoundVolume();
		static bool isMuted();
		static bool isCaptionVolumeActive();
		static void setMusicVolume(int volume);
		static void setSoundVolume(int volume);
		static void toggleMute();
		// Reexibe o titulo da janela com o nivel de volume atual.
		static void refreshCaption();
};

#endif /* AUDIO_H_ */
