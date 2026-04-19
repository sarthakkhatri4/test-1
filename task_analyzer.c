#include <stdio.h>
#include <string.h>
#include "task_analyzer.h"
#include "subject_manager.h"

struct Task tasks[MAX_TASKS];
int taskCount = 0;

void createTaskList()
{
    taskCount = 0;

    for(int i = 0; i < subjectCount; i++)
    {
        strcpy(tasks[taskCount].subjectName, subjects[i].name);

        if(strcmp(subjects[i].status, "Weak") == 0)
            tasks[taskCount].priority = 90;
       
         else if(strcmp(subjects[i].status, "Avarage") == 0)
            tasks[taskCount].priority = 75;

        else if(strcmp(subjects[i].status, "Moderate") == 0)
            tasks[taskCount].priority = 45;

        else
            tasks[taskCount].priority = 30;

        taskCount++;
    }

    printf("Task list created successfully!\n");
}

void sortTasksByPriority()
{
    for(int i = 0; i < taskCount - 1; i++)
    {
        for(int j = i + 1; j < taskCount; j++)
        {
            if(tasks[i].priority < tasks[j].priority)
            {
                struct Task temp = tasks[i];
                tasks[i] = tasks[j];
                tasks[j] = temp;
            }
        }
    }

    printf("Tasks sorted by priority!\n");
}

void viewTasks()
{
    if(taskCount == 0)
    {
        printf("No tasks available.\n");
        return;
    }

    printf("\nStudy Priority List:\n");

    for(int i = 0; i < taskCount; i++)
    {
        printf("%d. %s (Priority: %d)\n",
            i + 1,
            tasks[i].subjectName,
            tasks[i].priority);
    }
}