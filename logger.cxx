/*
 * Copyright (c) 2012 Vasily i. Redkin <vir@ctm.ru>
 * License: MIT (See LICENSE.txt or http://www.opensource.org/licenses/MIT)
 */
#include "logger.hpp"
#include "supervisor.hpp"
#include <sys/time.h>
#include <iostream>
#include <sstream>
#include <stdexcept>

Logger::Logger(const std::string & name)
	:m_name(name), m_append_date(true), m_usecs_timestamps(false)
	, m_append_pid(false)
{
}

Logger::~Logger()
{
	close();
}

void Logger::open()
{
	if(m_file.is_open())
		return;
	m_filename = super.logdir() + "/" + m_name + ".log";
	//m_file.exceptions ( std::ofstream::eofbit | std::ofstream::failbit | std::ofstream::badbit );
	m_file.open(m_filename.c_str(), std::ios::out | std::ios::app);
	if(m_file.fail())
		throw std::runtime_error(std::string("Can not open log file ") + m_filename);
}

void Logger::close()
{
	if(m_file.is_open())
		m_file.close();
}

static std::string now(bool usecs = false)
{
	char outstr[200];
	struct tm *tmp;
	const static char * format = "%Y-%m-%d %H:%M:%S";
	int len;
	time_t t;
	struct timeval tv;

	if(usecs) {
		gettimeofday(&tv, NULL);
		t = tv.tv_sec;
	} else {
		t = time(NULL);
	}

	tmp = localtime(&t);
	if (tmp == NULL) {
		perror("localtime");
		return "";
	}

	len = strftime(outstr, sizeof(outstr), format, tmp);
	if (len == 0) {
		fprintf(stderr, "strftime returned 0");
		return "";
	}

	if(usecs)
		sprintf(outstr+len, ".%06ld", tv.tv_usec);

	return std::string(outstr);
}

std::string Logger::log(const std::string& tag, const std::string msg, pid_t pid, bool forcetimestamp/* = false*/, bool forcepid/* = false*/)
{
	std::ostringstream ss;
	if(m_append_date || forcetimestamp)
		ss << now(m_usecs_timestamps) << " ";
	ss << m_name;
	if(m_append_pid || forcepid)
		ss << "[" << pid << "]";
	ss << "[" << tag << "] " << msg << std::endl;
	if(!super.background())
		std::cout << ss.str();
	m_file << ss.str();
	m_file.flush();
	return ss.str();
}

bool Logger::configure(const std::string & var, const std::string & value)
{
	//std::cout << "Logger::configure(" << var << ", " << value << ")" << std::endl;
	if(var == "timestamp") {
		m_append_date = Config::to_bool(value);
		return true;
	} else if(var == "microseconds") {
		m_usecs_timestamps = Config::to_bool(value);
		return true;
	} else if(var == "pid") {
		m_append_pid = Config::to_bool(value);
		return true;
	}
	return false;
}

