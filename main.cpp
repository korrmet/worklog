#include <cstdio>
#include <iostream>
#include <string>
#include <ctime>
#include <time.h>
#include <regex>
#include <cstring>
#include <fstream>
#include <list>
#include <map>

#define VERSION "v0.2"

namespace timestamp
{ time_t now() { return std::time(NULL); }

  std::string time2s(time_t t)
  { char buffer[80]; memset(buffer, 0, sizeof(buffer));
    strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", std::localtime(&t));
    return std::string(buffer); }

  time_t s2time(std::string s)
  { std::smatch match;
    struct tm t; memset(&t, 0, sizeof(t));
    time_t result = 0;

    if (std::regex_match(s, match,
                         std::regex("([0-9]{2})/([0-9]{2})/([0-9]{4}) "
                                    "([0-9]{2}):([0-9]{2}):([0-9]{2})")))
    { t.tm_mday = std::stoi(match[1].str());
      t.tm_mon  = std::stoi(match[2].str()) - 1;
      t.tm_year = std::stoi(match[3].str()) - 1900;
      t.tm_hour = std::stoi(match[4].str());
      t.tm_min  = std::stoi(match[5].str());
      t.tm_sec  = std::stoi(match[6].str());

      result = std::mktime(&t); }

    return result; }

  typedef unsigned long dmy_t;
  dmy_t s2dmy(std::string s)
  { std::smatch match; dmy_t dmy = 0;

    if (std::regex_match(s, match,
                         std::regex("([0-9]{2})/([0-9]{2})/([0-9]{4}) "
                                    "[0-9]{2}:[0-9]{2}:[0-9]{2}"))
        ||
        std::regex_match(s, match,
                         std::regex("([0-9]{2})/([0-9]{2})/([0-9]{4})")))
    { dmy += std::stoi(match[1].str());
      dmy += std::stoi(match[2].str()) * 31;
      dmy += std::stoi(match[3].str()) * 31 * 12; }

    return dmy; }

  dmy_t time2dmy(time_t t) { return s2dmy(time2s(t)); }

  std::string dmy2s(dmy_t dmy)
  { char buf[80]; memset(buf, 0, sizeof(buf));
    int y = dmy / (31 * 12); dmy %= 31 * 12;
    int m = dmy / 31;        dmy %= 31;
    int d = dmy;
    std::snprintf(buf, sizeof(buf), "%02d/%02d/%04d", d, m, y);
    return std::string(buf); }

  typedef unsigned long hms_t;

  hms_t s2hms(std::string s)
  { std::smatch match; hms_t hms = 0;

    if (std::regex_match(s, match,
                         std::regex("[0-9]{2}/[0-9]{2}/[0-9]{4} "
                                    "([0-9]{2}):([0-9]{2}):([0-9]{2})"))
        ||
        std::regex_match(s, match,
                         std::regex("([0-9]{2}):([0-9]{2}):([0-9]{2})"))
        ||
        std::regex_match(s, match,
                         std::regex("([0-9]{4}):([0-9]{2}):([0-9]{2})")))
    { hms += std::stoi(match[1].str()) * 60 * 60;
      hms += std::stoi(match[2].str()) * 60;
      hms += std::stoi(match[3].str()); }

    return hms; }

  std::string hms2s(hms_t hms)
  { char buf[80]; memset(buf, 0, sizeof(buf));
    int h = hms / (60 * 60); hms %= 60 * 60;
    int m = hms / 60;        hms %= 60;
    int s = hms;
    std::snprintf(buf, sizeof(buf), "%04d:%02d:%02d", h, m, s);
    return std::string(buf); }
};

#define CKARG(str) (std::string(argv[i]) == str)
#define CKNXT() (i + 1 < argc)

class cli_parser
{ public:
  cli_parser(int argc, char** argv)
    : path("default.worklog"),
      help(false), report(false),
      begin(0), end(0)
  { for (int i = 1; i < argc; i++)
    { if (CKARG("-h") || CKARG("--help")) { help = true; }
      if (CKNXT() && (CKARG("-f") || CKARG("--file"))) { i++; path = argv[i]; }
      if (CKARG("--report") || CKARG("-r")) { report = true; }
      if (CKNXT() && (CKARG("--date") || CKARG("-d")))
      { report = true; i++; begin = timestamp::s2dmy(argv[i]); end = begin; }
      if (CKNXT() && (CKARG("--begin") || CKARG("-b")))
      { report = true; i++; begin = timestamp::s2dmy(argv[i]); }
      if (CKNXT() && (CKARG("--end") || CKARG("-e")))
      { report = true; i++; end = timestamp::s2dmy(argv[i]); } }

    if (report && !begin && !end)
    { begin = timestamp::time2dmy(timestamp::now()); end = begin; }

    if (report && !end) { end = timestamp::time2dmy(timestamp::now()); }
  }

  std::string path;
  bool help, report;
  timestamp::dmy_t begin, end;

  const std::string help_str =
    "Usage: worklog (--file <filename>) (report opts)\n"
    "Options: [--file/-f]   specifies the file that would be used for storing\n"
    "                       log for this session. Optional, default file is\n"
    "                       \"default_worklog\"\n"
    "         [--report/-r] creates report, if no additional keys specified\n"
    "                       prints report for today\n"
    "         [--date/-d]   creates report for specific date DD/MM/YYYY\n"
    "         [--begin/-b]  specifies the start date of the series of reports\n"
    "                       DD/MM/YY. Optional, may be used without end date\n"
    "         [--end/-e]    specifies the end date of the series of reports\n"
    "                       DD/MM/YY. Optional, if this date not specified,\n"
    "                       it would be assigned for today\n"
    ;
};

class tui_parser
{ public:
  tui_parser() { clear(); }

#define MATCH(re) std::regex_match(input, match, std::regex(re))

  void parse(std::string input)
  { clear(); 
    if (input.empty()) { return; }

    std::smatch match;

    if      (MATCH(help_re)    || MATCH(h_re)) { help         = true; }
    else if (MATCH(version_re) || MATCH(v_re)) { version      = true; }
    else if (MATCH(quit_re)    || MATCH(q_re)) { quit         = true; }
    else if (MATCH(report_re)  || MATCH(r_re))
    { report = true; dmy_start = 0; dmy_end = 0; }
    else if (MATCH(report_d_re) || MATCH(r_d_re))
    { report = true;
      dmy_start = timestamp::s2dmy(match[1].str());
      dmy_end   = dmy_start; }
    else if (MATCH(report_i_re) || MATCH(r_i_re))
    { report = true;
      dmy_start = timestamp::s2dmy(match[1].str());
      dmy_end   = timestamp::s2dmy(match[2].str()); }
    else if (MATCH(short_re) || MATCH(s_re))
    { sreport = true; dmy_start = 0; dmy_end = 0; }
    else if (MATCH(short_d_re) || MATCH(s_d_re))
    { sreport = true;
      dmy_start = timestamp::s2dmy(match[1].str());
      dmy_end   = dmy_start; }
    else if (MATCH(short_i_re) || MATCH(s_i_re))
    { sreport = true;
      dmy_start = timestamp::s2dmy(match[1].str());
      dmy_end   = timestamp::s2dmy(match[2].str()); }
    else if (MATCH(command_re)) { error   = true; payload = match[1].str(); }
    else if (MATCH(task_re))    { task    = true; payload = match[1].str(); }
    else if (MATCH(comment_re)) { comment = true; payload = match[1].str(); } 
  }

  bool error, help, report, version, quit, task, comment, sreport;
  timestamp::dmy_t dmy_start, dmy_end;

  std::string payload;

  const std::string help_desc =
    "!<task name>                     - enter task name\n"
    "<anything>                       - comment line\n"
    ":help/:h                         - this help\n"
    ":report/:r [<start> <end>]       - make report for today/specific date\n"
    ":short report/:s [<start> <end>] - make short report\n"
    ":version/:v                      - print version of the program\n"
    ":quit/:q                         - exit the program\n";

  private:
  const std::string command_re  = ":(.*)";
  const std::string report_re   = ": *report *";
  const std::string report_d_re = ": *report"
                                  " *([0-9]{2}/[0-9]{2}/[0-9]{4}) *";
  const std::string report_i_re = ": *report"
                                  " *([0-9]{2}/[0-9]{2}/[0-9]{4})"
                                  " *([0-9]{2}/[0-9]{2}/[0-9]{4}) *";
  const std::string r_re        = ": *r";
  const std::string r_d_re      = ": *r"
                                  " *([0-9]{2}/[0-9]{2}/[0-9]{4}) *";
  const std::string r_i_re      = ": *r"
                                  " *([0-9]{2}/[0-9]{2}/[0-9]{4})"
                                  " *([0-9]{2}/[0-9]{2}/[0-9]{4}) *";
  const std::string short_re    = ": *short *report *";
  const std::string short_d_re  = ": *short *report *"
                                  " *([0-9]{2}/[0-9]{2}/[0-9]{4}) *";
  const std::string short_i_re  = ": *short *report"
                                  " *([0-9]{2}/[0-9]{2}/[0-9]{4}) *"
                                  " *([0-9]{2}/[0-9]{2}/[0-9]{4}) *";
  const std::string s_re        = ": *s *";
  const std::string s_d_re      = ": *s"
                                  " *([0-9]{2}/[0-9]{2}/[0-9]{4})";
  const std::string s_i_re      = ": *s"
                                  " *([0-9]{2}/[0-9]{2}/[0-9]{4}) *"
                                  " *([0-9]{2}/[0-9]{2}/[0-9]{4}) *";
  const std::string version_re  = ": *version *";
  const std::string v_re        = ": *v *";
  const std::string quit_re     = ":.*quit *";
  const std::string q_re        = ":.*q *";
  const std::string help_re     = ":.*help *";
  const std::string h_re        = ":.*h *";
  const std::string task_re     = "! *(.+) *";
  const std::string comment_re  = " *(.+) *";

  void clear()
  { error = false; help = false; version = false; quit = false; task = false;
    comment = false; report = false; sreport = false;
    dmy_start = 0; dmy_end = 0;
    payload.clear(); }
};

class log_file
{ public:
  log_file(std::string name) : name(name) {}

  void append(std::string s)
  { std::ofstream f(name, std::ios_base::app);
    if (!f) { return; }
    f << s << "\n";
    f.close(); }

  std::string last_line()
  { std::ifstream f(name);
    if (!f) { return ""; }
    f.seekg(-1, std::ios_base::end);

    bool last = true;
    while (true)
    { char ch; f.get(ch);
      if (!last && ch == '\n') { break; }
      else if ((int)f.tellg() <= 1) { f.seekg(0); break; }
      f.seekg(-2, std::ios_base::cur); last = false; }

    std::string ret; getline(f, ret); f.close(); return ret; }

  void rewind() { file.close(); file.open(name); }
  void close()  { file.close(); }

  std::string line()
  { std::string res;
    if (!file.is_open()) { file.open(name); }
    if (file.is_open()) { std::getline(file, res); }
    return res; }

  private:
  std::string name;
  std::ifstream file;
};

class core
{ public:
  core(log_file& lf) : lf(lf) {}

  void begin() { day_edge(); lf.append(record("begin")); }
  void end() { day_edge(); lf.append(record("end")); }
  void comment(std::string p) { day_edge(); lf.append(record("comment", p)); }
  void task(std::string p) { day_edge(); lf.append(record("task", p)); }

  std::string report(timestamp::dmy_t start = 0,
                     timestamp::dmy_t end = 0,
                     bool sreport = false)
  { if (!start) { start = timestamp::time2dmy(timestamp::now()); }
    if (!end)   { end   = timestamp::time2dmy(timestamp::now()); }

    std::string out;
    out.append("Report ").append(timestamp::dmy2s(start)).append(" - ")
                         .append(timestamp::dmy2s(end))  .append("\n");
    lf.rewind();

    std::string curr_task;
    std::list<std::string> curr_comments;
    unsigned int switches = 0;
    timestamp::hms_t elapsed  = 0;
    timestamp::hms_t hms_prev = 0;
    timestamp::hms_t hms_now  = 0;
    timestamp::dmy_t dmy_prev = 0;
    timestamp::dmy_t dmy_now  = 0;

    struct task_stuff
    { std::list<std::string> comments;
      timestamp::hms_t elapsed; };

    std::map<std::string, task_stuff> tasks;
    std::map<std::string, task_stuff> tasks_daily;

    while (true)
    { std::string rec = lf.line(); if (rec.empty()) { break; }
      if (!rec_match(rec, start, end)) { continue; }

      hms_now = timestamp::s2hms(get_rec_time(rec));
      timestamp::hms_t delta = hms_now - hms_prev;
      hms_prev = hms_now;

      dmy_now = timestamp::s2dmy(get_rec_time(rec));
      if (dmy_prev != dmy_now && !tasks_daily.empty())
      { timestamp::hms_t summary = 0;
        out.append("\n").append(timestamp::dmy2s(dmy_prev)).append("\n");
        for (std::map<std::string, task_stuff>::iterator i =
               tasks_daily.begin();
             i != tasks_daily.end(); i++)
        { summary += i->second.elapsed;
          out.append("Task ").append(i->first.c_str()).append(": ")
             .append(timestamp::hms2s(i->second.elapsed)).append("\n");
          if (sreport) { continue; }
          for (std::string comment : i->second.comments)
          { out.append("  - ").append(comment).append("\n"); } }
        out.append("Summary: ").append(timestamp::hms2s(summary)).append("\n");
        out.append("Extra switches: ")
           .append(std::to_string(switches - tasks_daily.size())).append("\n");
        tasks_daily.clear(); switches = 0; }
      dmy_prev = dmy_now;

      switch (get_rec_type(rec))
      { case rec_type::begin:
        { curr_task.clear(); elapsed = 0; hms_prev = hms_now;
        } break;
        
        case rec_type::end:
        { elapsed += delta;

          if (!curr_task.empty())
          { if (!tasks.count(curr_task))
            { tasks[curr_task].comments = curr_comments;
              tasks[curr_task].elapsed = elapsed; }
            else
            { for (std::string comment : curr_comments)
              { tasks[curr_task].comments.push_back(comment); }
              tasks[curr_task].elapsed += elapsed; }
          
            if (!tasks_daily.count(curr_task))
            { tasks_daily[curr_task].comments = curr_comments;
              tasks_daily[curr_task].elapsed = elapsed; }
            else
            { for (std::string comment : curr_comments)
              { tasks_daily[curr_task].comments.push_back(comment); }
              tasks_daily[curr_task].elapsed += elapsed; } }

          elapsed = 0; curr_task.clear(); curr_comments.clear();
        } break;
        
        case rec_type::task:
        { elapsed += delta; switches++;

          if (!curr_task.empty())
          { if (!tasks.count(curr_task))
            { tasks[curr_task].comments = curr_comments;
              tasks[curr_task].elapsed = elapsed; }
            else
            { for (std::string comment : curr_comments)
              { tasks[curr_task].comments.push_back(comment); }
              tasks[curr_task].elapsed += elapsed; }

          if (!tasks_daily.count(curr_task))
          { tasks_daily[curr_task].comments = curr_comments;
            tasks_daily[curr_task].elapsed = elapsed; }
          else
          { for (std::string comment : curr_comments)
            { tasks_daily[curr_task].comments.push_back(comment); }
            tasks_daily[curr_task].elapsed += elapsed; } }

          elapsed = 0; curr_task = rec_payload(rec); curr_comments.clear();
        } break;
        
        case rec_type::comment:
        { elapsed += delta; curr_comments.push_back(rec_payload(rec));
        } break;
        
        default: break; } }

    if (!curr_comments.empty() && !curr_task.empty())
    { if (!tasks.count(curr_task))
      { tasks[curr_task].comments = curr_comments;
        tasks[curr_task].elapsed = elapsed; }
      else
      { for (std::string comment : curr_comments)
        { tasks[curr_task].comments.push_back(comment); }
        tasks[curr_task].elapsed += elapsed; }

      if (!tasks_daily.count(curr_task))
      { tasks_daily[curr_task].comments = curr_comments;
        tasks_daily[curr_task].elapsed = elapsed; }
      else
      { for (std::string comment : curr_comments)
        { tasks_daily[curr_task].comments.push_back(comment); }
        tasks_daily[curr_task].elapsed += elapsed; } }

    if (!tasks_daily.empty())
    { timestamp::hms_t summary = 0;
      out.append("\n").append(timestamp::dmy2s(dmy_now)).append("\n");
      for (std::map<std::string, task_stuff>::iterator i =
             tasks_daily.begin();
           i != tasks_daily.end(); i++)
      { summary += i->second.elapsed;
        out.append("Task ").append(i->first.c_str()).append(": ")
           .append(timestamp::hms2s(i->second.elapsed)).append("\n");
        if (sreport) { continue; }
        for (std::string comment : i->second.comments)
        { out.append("  - ").append(comment).append("\n"); } }
      out.append("Summary: ").append(timestamp::hms2s(summary)).append("\n");
      out.append("Extra switches: ")
         .append(std::to_string(switches - tasks_daily.size())).append("\n");
      tasks_daily.clear(); switches = 0; }

    if (start != end)
    { out.append("\nTOTAL\n");
      for (std::map<std::string, task_stuff>::iterator i = tasks.begin();
           i != tasks.end(); i++)
      { out.append("Task ").append(i->first.c_str()).append(": ")
           .append(timestamp::hms2s(i->second.elapsed)).append("\n");
        if (sreport) { continue; }
        for (std::string comment : i->second.comments)
        { out.append("  - ").append(comment).append("\n"); } } }

    return out; }

  private:

  enum class rec_type { begin, end, task, comment, unknown };

  rec_type get_rec_type(std::string s)
  { rec_type res = rec_type::unknown;
    
    std::smatch match;
    if (std::regex_match(s, match, std::regex("\\[.+\\]\\[begin\\]")))
    { res = rec_type::begin; }
    else if (std::regex_match(s, match, std::regex("\\[.+\\]\\[end\\]")))
    { res = rec_type::end; }
    else if (std::regex_match(s, match,
                              std::regex("\\[.+\\]\\[task\\]\\[.+\\]")))
    { res = rec_type::task; }
    else if (std::regex_match(s, match,
                              std::regex("\\[.+\\]\\[comment\\]\\[.+\\]")))
    { res = rec_type::comment; }

    return res; }

  std::string get_rec_time(std::string s)
  { std::string res; std::smatch match;
    if (std::regex_match(s, match, std::regex("\\[(.+)\\]\\[.+\\]\\[.+\\]"))
        ||
        std::regex_match(s, match, std::regex("\\[(.+)\\]\\[.+\\]")))
    { res = match[1].str(); }
    return res; }

  void day_edge()
  { std::string line = lf.last_line();
    std::smatch match;
    if (std::regex_match(line, match, std::regex("\\[(.+)\\]\\[(.+)\\]\\[.+\\]"))
        ||
        std::regex_match(line, match, std::regex("\\[(.+)\\]\\[(.+)\\]")))
    { if (match[2].str() != "end")
      { timestamp::dmy_t last = timestamp::s2dmy(match[1].str());
        timestamp::dmy_t now  = timestamp::time2dmy(timestamp::now());
        if (last != now)
        { std::string rec_last;
          rec_last.append("[").append(timestamp::dmy2s(last))
                  .append(" 23:59:59][end]");
          lf.append(rec_last); } } } }

  std::string rec_payload(std::string rec)
  { std::string result; std::smatch match;
    if (std::regex_match(rec, match, std::regex("\\[.+\\]\\[.+\\]\\[(.+)\\]")))
    { result = match[1].str(); }
    return result; }

  bool rec_match(std::string rec, timestamp::dmy_t start, timestamp::dmy_t end)
  { std::smatch match;
    timestamp::dmy_t dmy;
    if (std::regex_match(rec, match, std::regex("\\[(.+)\\]\\[.+\\]\\[.+\\]"))
        ||
        std::regex_match(rec, match, std::regex("\\[(.+)\\]\\[.+\\]")))
    { dmy = timestamp::s2dmy(match[1].str());
      if (dmy >= start && dmy <= end) { return true; } }
    return false; }

  std::string record(std::string type, std::string pload = "")
  { std::string res;
    res.append("[").append(timestamp::time2s(timestamp::now())).append("]");
    if (!type.empty()) { res.append("[").append(type).append("]"); }
    if (!pload.empty()) { res.append("[").append(pload).append("]"); }
    return res; }

  log_file& lf;
};

int main(int argc, char** argv)
{ std::printf("Worklog %s\n", VERSION);

  cli_parser cp(argc, argv);

  if (cp.help) { std::printf("%s\n", cp.help_str.c_str()); return 0; }

  std::printf("Using \"%s\" log file\n", cp.path.c_str());
  log_file lf(cp.path);
  core c(lf);

  if (cp.report)
  { std::printf("%s\n", c.report(cp.begin, cp.end).c_str());
    return 0; }

  std::printf("Type \":help\" to get help and \":quit\" to exit the program\n");

  c.begin();
  tui_parser tp;
  std::string curr_task;
  while (true)
  { std::printf("%s > ", curr_task.c_str());
    std::string line; std::getline(std::cin, line); tp.parse(line);
    if (tp.error)        { std::printf("Error: %s\n", tp.payload.c_str());
                           std::printf("Use \":help\" to get help\n"); }
    else if (tp.help)    { std::printf("Available commands:\n%s",
                                       tp.help_desc.c_str()); }
    else if (tp.version) { std::printf("%s\n", VERSION); }
    else if (tp.quit)    { c.end(); break; }

    else if (tp.report)  { std::printf("%s", c.report(tp.dmy_start,
                                                      tp.dmy_end).c_str()); }
    else if (tp.sreport) { std::printf("%s", c.report(tp.dmy_start,
                                                      tp.dmy_end, 1).c_str()); } 
    else if (tp.task)    { c.task(tp.payload); curr_task = tp.payload; }
    else if (tp.comment) { c.comment(tp.payload); } }

  return 0; }
