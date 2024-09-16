# worklog

Simple cli work time tracker. No extra bullshit. Just bare minimum. Convenient
and fast. Not require a lot of 

# Features

- Reporting tasks for period and the time spent
- Reporting all of the actions you record for task
- Reporting how often you switch between tasks

# Usage

Just type what you're doing and all of this would append to the log.
First enter the current task you working at, and then anything you enter would
be recognized as comments to that task. To start doing another task just enter
new name of the task and keep record what you do.

To start doing task just type '!' at the very beginning, rest of your input
would be the name of the new task.

Worklog has vi-like commands, type ":help" to explore them.

# Example

```
Worklog v0.2
Using "default_worklog" log file
Type ":help" to get help and ":quit" to exit the program
 > !Important task
Important task > I did this
Important task > I did that
Important task > !Another important task
Another important task > I did this
Another important task > I did that
Another important task > :report
Report 17/09/2024 - 17/09/2024

17/09/2024
Task Another important task: 0000:00:13
  - I did this
  - I did that
Task Important task: 0000:00:25
  - I did this
  - I did that
Summary: 0000:00:38
Extra switches: 0
Another important task > 
```
