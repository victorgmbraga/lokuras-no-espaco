/*
 * main.cpp
 *
 *  Created on: 16/04/2013
 *      Author: Victor
 */

#include "StateManager.h"

int main(int argc, char *argv[])
{
	StateManager *state = new StateManager();
	state->run();
	delete(state);
	return 0;
}



