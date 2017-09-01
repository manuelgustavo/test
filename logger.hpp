#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <thread_tools/threadex.hpp>
#include <thread_tools/queuer.hpp>
#include <thread_tools/timetools.hpp>

#include <string>
#include <fstream>
#include <ostream>
#include <ctime>
#include <memory>
#include <array>



namespace thread_tools {
	// asynchronous (almost) non-blocking text file logger
	
	struct LogMessage {
		enum level {
			DEBUG,
			INFORMATION,
			WARNING,
			ERROR,
			LEVEL_END
		};// type;

		std::string message;
	};

	static const std::array<const char*, LogMessage::LEVEL_END> level_s = {
		"dbg",
		"inf",
		"wrn",
		"err"
	};

	class AsyncLogger : public Queuer<LogMessage, AsyncLogger>{
        using Queuer<LogMessage, AsyncLogger>::Push;

        std::string file_name_;
        std::ofstream file_;

	public:

		AsyncLogger(const decltype(file_name_)& file_name, bool truncate = false) : file_name_{ file_name }  {
			file_.open(file_name, std::ofstream::out | (truncate ? std::ofstream::trunc : std::ofstream::app));
			if (!file_.is_open()) {
				throw std::runtime_error("logger::logger : couldn't open file");
			}
			
		}
		
		~AsyncLogger() {
			StopQueuer();
			if (file_.is_open())
				file_.close();
		}

		void operator()(std::unique_ptr<LogMessage> p) {
			file_ << p->message;
		}

		void Log(const std::string& message, const LogMessage::level type = LogMessage::DEBUG) {
			auto now = std::chrono::system_clock::now();

			std::tm tm_lt = *LOCAL_TM(now);
			std::tm tm_utc = *UTC_TM(now);

			std::string buff(38 + sizeof("U.") - 1 + sizeof("L.") - 1 + sizeof("[") - 1 + 3 + sizeof("] ") + message.length(), '\n');

			TmToString(tm_utc, &buff[0]);
			buff[19] = 'U'; buff[20] = '.';
			TmToString(tm_lt, &buff[21]);
			buff[40] = 'L'; buff[41] = '.';
			buff[42] = '['; 
			memcpy_s(&buff[43], 3, level_s[type], 3);
			buff[46] = ']'; buff[47] = ' ';

			memcpy_s(&buff[48], 38 + sizeof("U.") - 1 + sizeof("L.") - 1 + sizeof("[") - 1 + sizeof("] ") + message.length(), &message[0], message.size());

            Push(std::make_unique<LogMessage>(LogMessage{ std::move(buff) }));
		}

	};
}

#endif // LOGGER_HPP