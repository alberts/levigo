package levigo

//#cgo linux LDFLAGS: -lstdc++
//#include "leveldb/c.h"
//leveldb_logger_t* pipe_logger(int fd);
//extern void free_pipe_logger(leveldb_logger_t* logger);
import "C"

import (
	"bufio"
	"io"
	"os"
)

type Logger struct {
	logger *C.leveldb_logger_t
}

func PipeLogger(writer io.Writer) *Logger {
	r, w, err := os.Pipe()
	if err != nil {
		return nil
	}
	logger := C.pipe_logger(C.int(w.Fd()))
	if logger == nil {
		r.Close()
		w.Close()
		return nil
	}

	// logger owns w now.
	// freeing the logger will close w.

	go func() {
		defer r.Close()
		reader := bufio.NewReader(r)
		for {
			line, err := reader.ReadBytes('\n')
			if err != nil {
				return
			}
			_, err = writer.Write(line)
			if err != nil {
				return
			}
		}
	}()
	return &Logger{logger}
}

func FreePipeLogger(logger *Logger) {
	C.free_pipe_logger(logger.logger)
}
