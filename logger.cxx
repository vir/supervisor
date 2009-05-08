#include "logger.hpp"
#include "supervisor.hpp"
#include <iostream>
#include <sstream>

Logger::Logger(const std::string & name)
	:m_name(name), m_append_date(true)
{
	m_filename = super.logdir() + "/" + m_name + ".log";
	m_file.exceptions ( std::ofstream::eofbit | std::ofstream::failbit | std::ofstream::badbit );
	m_file.open(m_filename.c_str(), std::ios::out | std::ios::app);
}

Logger::~Logger()
{
	m_file.close();
}

static std::string now()
{
	char outstr[200];
	time_t t;
	struct tm *tmp;
	const static char * format = "%Y-%m-%d %H:%M:%S";

	t = time(NULL);
	tmp = localtime(&t);
	if (tmp == NULL) {
		perror("localtime");
		return "";
	}

	if (strftime(outstr, sizeof(outstr), format, tmp) == 0) {
		fprintf(stderr, "strftime returned 0");
		return "";
	}
	return std::string(outstr);
}

std::string Logger::log(const std::string & source, std::string tag, const std::string msg, bool forcetimestamp)
{
	std::ostringstream ss;
//	std::cout << "Log[" << m_name << "](" << source << ", " << tag << ", " << msg << ", " << forcetimestamp << ")" << std::endl;
	if(m_append_date || forcetimestamp)
		ss << now() << " ";
	ss << source << "[" << tag << "] " << msg << std::endl;
	if(1)
		std::cout << ss.str();
	m_file << ss.str();
	m_file.flush();
	return ss.str();
}

bool Logger::configure(const std::string & var, const std::string & value)
{
	std::cout << "Logger::configure(" << var << ", " << value << ")" << std::endl;
	if(var == "timestamp") {
		m_append_date = Config::to_bool(value);
		return true;
	}
	return false;
}

