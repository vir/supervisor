#ifndef PIDFILE_HPP_INCLUDED
#define PIDFILE_HPP_INCLUDED

class Pidfile
{
	private:
		char * m_path;
	public:
		Pidfile(const char * path);
		~Pidfile();
};

#endif /* PIDFILE_HPP_INCLUDED */

