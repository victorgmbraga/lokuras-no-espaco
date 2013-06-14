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
	public:
		Audio(string fileName, int type);
		virtual ~Audio();
		void play(int n);
		void stop();
};

#endif /* AUDIO_H_ */
