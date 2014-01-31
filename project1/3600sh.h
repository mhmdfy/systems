/*
 * CS3600, Spring 2013
 * Project 1 Starter Code
 * (c) 2013 Alan Mislove
 *
 * You should use this (very simple) starter code as a basis for
 * building your shell.  Please see the project handout for more
 * details.
 */

#ifndef _3600sh_h
#define _3600sh_h

#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

void getargs(char* cmd, int *argcp, char **argv);

char* getword(char * begin, char **endp);

void redirect(char* argv[]);

int validRedirect(char* file);

void background(char* argv[]);

char escape(char c);

void execute(char *argv[]);

void do_exit();

#endif 
