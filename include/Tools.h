/*
 * Tools.h
 *
 *  Created on: Nov 7, 2023
 *      Author: mhn
 */

#ifndef TOOLS_H_
#define TOOLS_H_

#include "main.h"


/***** PROTOTYPES *****/
int 			Servers_Start	(int trace)	;
void 			Servers_Monitor (void)		;
const char * 	get_time		(void)		;
void 			LED_Init 		(void)		;
void 			File_Init 		(char* argv[])	;
int 			report 			(REPORT_INTERFACE Interface, REPORT_MESSAGE Message, char *Information);

#endif /* TOOLS_H_ */
