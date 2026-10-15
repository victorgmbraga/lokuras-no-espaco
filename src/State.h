/*
 * State.h
 *
 *  Created on: 13/06/2013
 *      Author: Victor
 */

#ifndef STATE_H_
#define STATE_H_

class State
{
	public:
		State() {}
		virtual ~State() {}
		virtual void load(int stack = 0) = 0;
		virtual int unload() = 0;
		virtual int update(int dt) = 0;
		virtual void render() = 0;
};

#endif /* STATE_H_ */
