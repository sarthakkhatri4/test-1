#ifndef TASK_ANALYZER_H
#define TASK_ANALYZER_H

#define MAX_TASKS 110

struct Task
{
    char subjectName[60];
    int priority;   // Higher = more important
};

// Functions
void createTaskList();
void sortTasksByPriority();
void viewTasks();

#endif