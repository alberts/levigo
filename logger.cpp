#include "leveldb/c.h"
#include "leveldb/env.h"
#include <unistd.h>
#include <stdio.h>

class PipeLogger : public leveldb::Logger {
public:
	PipeLogger(int fd) : fd_(fd) {}
	virtual void Logv(const char* format, va_list ap);
	virtual ~PipeLogger() { ::close(fd_); }
private:
	int fd_;
};

void PipeLogger::Logv(const char* format, va_list ap) {
	char buf[512];
	int n;
	n = vsnprintf(buf, sizeof(buf), format, ap);
	if (n < 0) {
		return;
	}
	if (size_t(n)==sizeof(buf)) {
		// terminate with newline
		buf[sizeof(buf)-1] = '\n';
	}
	write(fd_, buf, n);
}

#ifdef __cplusplus
extern "C" {
#endif

struct leveldb_logger_t { leveldb::Logger* rep; };

extern leveldb_logger_t* pipe_logger(int fd) {
  leveldb_logger_t* logger = new leveldb_logger_t;
  logger->rep = new PipeLogger(fd);
  return logger;
}

extern void free_pipe_logger(leveldb_logger_t* logger) {
	delete logger->rep;
	delete logger;
}

#ifdef __cplusplus
}
#endif
