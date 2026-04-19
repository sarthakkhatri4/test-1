#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <stdio.h>
#define MAX_DAYS          7
#define MAX_SLOTS_PER_DAY 20   
#define MAX_NIGHT_STUDY_HRS 7.0f
#define BREAK_AFTER_MINS  105   
#define BREAK_DURATION    10                           

#define HARD_START_MIN   1080
#define MEDIUM_START_MIN 1260
#define EASY_START_MIN   1440
#define NIGHT_END_MIN    1560


struct ScheduleSlot
{
    int  startMin;
    int  endMin;
    char activity[160];
    int  difficulty;
};

struct DayPlan
{
    int                 dayNumber;
    struct ScheduleSlot slots[MAX_SLOTS_PER_DAY];
    int                 slotCount;
    float               totalStudyHours;
};

struct Subject;
extern int subjectCount;

struct Assignment;
extern int assignmentCount;

struct Task;
extern int taskCount;


void generateNightStudyPlan(void);


void generateWeeklySchedule(void);


void minToTimeStr(int minutes, char *out);

const char *diffLabel(int d);

const char *funMessage(void);

int addSlot(struct DayPlan *day,
            int startMin,
            int durationMins,
            const char *activity,
            int difficulty);

void printDay(const struct DayPlan *day, FILE *fp);

#endif 