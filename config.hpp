#ifndef CONFIG_HPP_INCLUDED
#define CONFIG_HPP_INCLUDED

#include <string>
#include <fstream>
#include <map>
#include <sstream> // for to_int

class ConfTarget
{
	public:
		virtual bool configure(const std::string & var, const std::string & value)=0;
		virtual ConfTarget * confcontext(const std::string & ctx, bool brackets);
};

class Config
{
	private:
		std::map<std::string, ConfTarget *> m_contexts;
		std::string m_curctx;
		ConfTarget * m_curtarget;
		ConfTarget * m_deftarget;
	public:
		Config():m_curtarget(0), m_deftarget(0) { }
		void register_context(const std::string & context, ConfTarget * target) { m_contexts[context] = target; }
		void unregister_context(const std::string & context) { m_contexts.erase(context); }
		void default_context(ConfTarget * target) { m_deftarget = target; if(!m_curtarget) m_curtarget = m_deftarget; }
		bool read_file(const std::string & fname);
		bool parse_line(std::string line);
		bool change_context(const std::string & context, bool autocreatefamily = false);
		void reset_context() { m_curtarget = m_deftarget; m_curctx.clear(); }
		static bool to_bool(const std::string & s) { return s == "true" || s == "yes" || s == "on" || s == "1"; }
		static int to_int(const std::string & s) { int r; std::stringstream ss(s); ss >> r; return r; }
};

extern Config config;

#endif /* CONFIG_HPP_INCLUDED */

