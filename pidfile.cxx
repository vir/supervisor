/*
 * Copyright (c) 2012 Vasily i. Redkin <vir@ctm.ru>
 * License: MIT (See LICENSE.txt or http://www.opensource.org/licenses/MIT)
 */
#include "pidfile.hpp"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


Pidfile::Pidfile(const char * path):m_path(0)
{
	if(!path || !*path)
		return;
	FILE * f = fopen(path, "wt");
	if(f) {
		m_path = strdup(path);
		fprintf(f, "%d\n", getpid());
		fclose(f);
	}
}

Pidfile::~Pidfile()
{
	if(m_path) {
		unlink(m_path);
		free(m_path);
	}
}


