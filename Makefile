run: clean worklog
	./worklog

worklog:
	g++ main.cpp -o worklog

clean:
	rm -rf worklog

clean_all: clean
	rm -rf default_worklog
