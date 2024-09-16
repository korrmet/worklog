run: clean worklog
	./worklog

worklog:
	g++ main.cpp -o worklog

install: worklog
	cp worklog /usr/bin

clean:
	rm -rf worklog

clean_all: clean
	rm -rf default.worklog
