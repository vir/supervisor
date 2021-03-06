/*
 * Copyright (c) 2012 Vasily i. Redkin <vir@ctm.ru>
 * License: MIT (See LICENSE.txt or http://www.opensource.org/licenses/MIT)
 */
#include "config.hpp"
#include "supervisor.hpp"
#include <iostream>
#include <dirent.h>
#include <unistd.h>

ConfTarget * ConfTarget::confcontext(const std::string & ctx, bool brackets)
{
	return NULL;
}

Config config;

void trim(std::string & str)
{
	const static char * whitespace = " \t\r\n";
	std::string::size_type pos = str.find_last_not_of(whitespace);
	if(pos != std::string::npos) {
		str.erase(pos + 1);
		pos = str.find_first_not_of(whitespace);
		if(pos != std::string::npos) str.erase(0, pos);
	} else
		str.erase(str.begin(), str.end());
}

bool Config::load_confdir(const std::string & path)
{
	DIR * d = opendir(path.c_str());
	if(! d) {
		perror(path.c_str());
		return false;
	}
	dirent * e;
	while((e = readdir(d))) {
		if(e->d_type != DT_REG)
			continue;
		std::string p(path);
		if(*p.rbegin() != '/')
			p += '/';
		p += e->d_name;
		if(p.length() > 5 && p.substr(p.length() - 5) != ".conf")
			continue;
		if(0 != access(path.c_str(), R_OK))
			continue;
		reset_context();
		if(! read_file(p))
			return false;
	}
	closedir(d);
	return true;
}

bool Config::parse_line(std::string line)
{
	trim(line);
	if(!line.length() || line[0] == '#' || line[0] == ';')
		return true;
	if(line[0] == '[' && line[line.length() - 1] == ']') {
		std::string context = line.substr(1, line.length() - 2);
		trim(context);
		reset_context();
		return change_context(context, true);
	}
	if(line.substr(0, 8) == "confdir ") {
		size_t pos = line.find_first_not_of(" \t", 8);
		std::string path = line.substr(pos);
		trim(path);
		return load_confdir(path);
	}
	size_t eqpos = line.find('=');
	if(eqpos == std::string::npos)
		return false; // no =
	std::string key = line.substr(0, eqpos);
	std::string val = line.substr(eqpos+1);
	trim(key);
	trim(val);
	//std::cout << "Config line: '" << key << "' = '" << val << "'" << std::endl;
	ConfTarget * save = m_curtarget;
	std::string save2 = m_curctx; // XXX ??? may be drop it ???
	bool retval = false;
	for(;;) {
		size_t pos = key.find('.');
		if(pos == std::string::npos)
			break;
		if(!change_context(key.substr(0, pos)))
			return false;
		key.erase(0, pos+1);
	}
	if(m_curtarget)
		retval = m_curtarget->configure(key, val);
	m_curtarget = save;
	m_curctx = save2;
	return retval;
}

bool Config::read_file(const std::string & fname)
{
	std::ifstream f(fname.c_str());
	std::string line;
	bool result = true;
	int linenum = 1;
	if(!f)
		return false;
	while(getline(f, line)) {
		if(!parse_line(line)) {
			std::cerr << "Error in " << fname << " line " << linenum << ": " << line << std::endl;
			result = false;
		}
		linenum++;
	}
	reset_context();
	return result;
}

bool Config::change_context(const std::string & context, bool brackets)
{
	std::map<std::string, ConfTarget *>::iterator it = m_contexts.find(context);
	if(it != m_contexts.end()) {
		m_curctx = it->first;
		m_curtarget = it->second;
		return true;
	}

	ConfTarget * newctx = m_curtarget->confcontext(context, brackets);
	if(newctx) {
		//std::cout << "Config context changed to " << context << std::endl;
		m_curctx = context; // XXX last part only
		m_curtarget = newctx;
		return true;
	} else {
		//std::cout << "Can not change config context to " << context << std::endl;
		return false;
	}
}

