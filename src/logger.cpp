#include <cstdarg>
#include <cstddef>
#include <vector>
#include <fstream>
#include "logger.hpp"

namespace tslogger
{

bool Handler::s_init = false;
std::mutex Handler::s_mutex;

time_t timestamp()
{
	return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

void timestamp_to_date_time_string(time_t ts, std::string &out)
{
	char buf[20];
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime( &ts ));
	out = buf;
}

void add_timestamp_prefix(const char *filename, std::string &out)
{
	time_t ts = timestamp();
	timestamp_to_date_time_string(ts, out);
	out += filename;
}

const char *log_level_to_string(log_level_t level)
{
	switch(level)
	{
	case ERROR:
		return "ERROR";
	case WARNING:
		return "WARNING";
	case INFO:
		return "INFO";
	case DEBUG:
		return "DEBUG";
	default:
		return "Unknown";
	}
}

void Logger::log(log_level_t level, const char *fmt, ...)
{
	Message msg;
	msg.timestamp = timestamp();
	msg.logLevel = level;
	msg.threadId = m_threadId;
	msg.filename = m_filename;
	msg.format = m_format;
	
    std::va_list args;
    va_start(args, fmt);

	for (const char *s = fmt; *s != '\0'; ++s) // list all specifiers 
	{
		switch(*s)
		{
		case '%':
			switch(*++s) // read format symbol
			{
			case 'd':
			case 'i':
				msg.message += std::to_string(va_arg(args, int)); // signed decimal integer
				continue;
			case 'F':
			case 'f':
				msg.message += std::to_string(va_arg(args, double)); // decimal floating point
				continue;
			case 's': 
				msg.message += std::string(va_arg(args, const char*)); // string of characters
				continue;
			case 'c':
				msg.message += static_cast<char>(va_arg(args, int)); // character
				continue;
			case '%':
				msg.message += "%"; // special character
				continue;
			case 'x':
			case 'X':
				msg.message += to_hex_string(va_arg(args, unsigned int)); // unsigned hexadecimal integer
				continue;
			case 'u':
				msg.message += std::to_string(va_arg(args, unsigned int)); // unsigned decimal integer
				continue;
			}
		case '\n':
			msg.message += "\n"; // special character
			continue;
		case '\t':
			msg.message += "\t"; // special character
			continue;
		default:
			msg.message += *s;
		}
	}
	va_end(args);
	m_queuePtr.get()->push(msg);
}

Handler::Handler(
		const char *root,
		log_level_t maxLevel,
		std::error_code &ec
	):
	m_root{root},
	m_maxLevel{maxLevel},
	m_queuePtr{std::make_shared<SafeQueue<Message>>()}
{
	if (root == nullptr) {
    	ec = make_system_error(EFAULT);
    	return;
	}
	if (s_init) {
		ec = make_error_code(TsLoggerStatus::TS_LOGGER_ERR_SINGLE_INSTANCE);
		return;
	}

	std::filesystem::create_directories(root);

	if (!std::filesystem::is_directory(m_root)) {
		ec = make_error_code(TsLoggerStatus::TS_LOGGER_ERR_NOT_DIRECTORY);
		return;		
	}

	s_init = true;
	ec.clear();
}

void Handler::process()
{
	m_queuePtr.get()->wait_wail_empty_for(1); // wait for 1 second
	Message msg = m_queuePtr.get()->front();
	m_queuePtr.get()->pop();

	std::filesystem::path filePath = m_root / msg.filename;

	std::ofstream ofs(filePath, std::ofstream::out | std::ofstream::app);

    if (ofs.is_open() && msg.logLevel <= m_maxLevel)
    {
    	if (msg.format & (1 << LEVEL_BIT))
    		ofs << "[" << log_level_to_string(msg.logLevel) << "] ";
    	if (msg.format & (1 << TIMESTAMP_BIT)) {
    		std::string ts;
    		timestamp_to_date_time_string(msg.timestamp, ts);
    		ofs << ts << " ";
    	}
    	if (msg.format & (1 << THREAD_ID_BIT)) {
    		std::stringstream ss;
    		ss << msg.threadId; 		
    		ofs << "thread_id : " << ss.str() << " ";
    	}
		ofs << msg.message;    	
        ofs.close();
    }
}

}
