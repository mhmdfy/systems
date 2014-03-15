/*
 * CS3600, Spring 2014
 * Project 2 Starter Code
 * (c) 2013 Alan Mislove
 *
 */

#ifndef __3600DNS_H__
#define __3600DNS_H__

char* FLAG;
char* SERVER;
short PORT;
char* NAME;

void processArgs(int argc, char* argv[]);
void processFlag(char* flag);
void processServerPort(char* serverPort);

#endif

