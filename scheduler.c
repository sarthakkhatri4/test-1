#include "scheduler.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


struct Assignment
{
    char  name[100];
    char  deadline[20];
    float timeRequired;
    int   isCompleted;
};

struct Subject
{
    char  name[100];
    float quizPercent;
    float minorPercent;
    int   confidence;
    float finalScore;
    char  status[20];
};

struct Task
{
    char title[100];
    int  priority;
};

extern struct Assignment assignments[];
extern int               assignmentCount;

extern struct Subject    subjects[];
extern int               subjectCount;

extern struct Task       tasks[];
extern int               taskCount;

extern void createTaskList(void);
extern void sortTasksByPriority(void);



static const char *funMsgs[] = {
    "Maze karo! Keep going!",
    "Moj karo! You earned it!",
    "Focus mode ON! Crush it!",
    "You are doing great — keep pushing!",
    "Padhle bhai, Netflix baad mein.",
    "Future self will thank you!",
    "One step closer to success!",
    "Khatam kar phir so jaana.",
    "No shortcuts to excellence!",
    "Consistency beats intensity every time!"
};
#define FUN_MSG_COUNT 10

const char *funMessage(void)
{
    return funMsgs[rand() % FUN_MSG_COUNT];
}


void minToTimeStr(int minutes, char *out)
{
    int normalized = ((minutes % 1440) + 1440) % 1440;
    int h = (normalized / 60) % 24;
    int m = normalized % 60;
    snprintf(out, 6, "%02d:%02d", h, m);
}

const char *diffLabel(int d)
{
    switch (d)
    {
        case 1:  return "Hard  ";
        case 2:  return "Medium";
        case 3:  return "Easy  ";
        default: return "Break ";
    }
}

static int subjectDiff(int idx)
{
    const char *st = subjects[idx].status;
    if (st == NULL || st[0] == '\0') return 3;

    if      (strcmp(st, "Weak")     == 0) return 1;
    else if (strcmp(st, "Moderate") == 0) return 2;
    else                                  return 3;
}

static int bandEnd(int diff)
{
    switch (diff)
    {
        case 1:  return MEDIUM_START_MIN;
        case 2:  return EASY_START_MIN;
        default: return NIGHT_END_MIN;
    }
}


int addSlot(struct DayPlan *day,
            int startMin,
            int durationMins,
            const char *activity,
            int difficulty)
{
    int i;

    if (day == NULL)
    {
        fprintf(stderr, "  [Scheduler] addSlot: NULL DayPlan pointer.\n");
        return 0;
    }
    if (durationMins <= 0)
    {
        fprintf(stderr, "  [Scheduler] addSlot: invalid duration %d – slot skipped.\n",
                durationMins);
        return 0;
    }
    if (activity == NULL)
    {
        fprintf(stderr, "  [Scheduler] addSlot: NULL activity string – slot skipped.\n");
        return 0;
    }
    if (day->slotCount >= MAX_SLOTS_PER_DAY)
    {
        fprintf(stderr,
                "  [Scheduler] Day %d: slot table full (%d slots) – '%s' skipped.\n",
                day->dayNumber, MAX_SLOTS_PER_DAY, activity);
        return 0;
    }

    i = day->slotCount;
    day->slots[i].startMin   = startMin;
    day->slots[i].endMin     = startMin + durationMins;
    day->slots[i].difficulty = difficulty;
    strncpy(day->slots[i].activity, activity, 159);
    day->slots[i].activity[159] = '\0';

    if (difficulty != 0)
        day->totalStudyHours += durationMins / 60.0f;

    day->slotCount++;
    return 1;
}


static void printSeparator(char ch, int width, FILE *fp)
{
    int i;
    for (i = 0; i < width; i++) putchar(ch);
    putchar('\n');
    if (fp)
    {
        for (i = 0; i < width; i++) fputc(ch, fp);
        fputc('\n', fp);
    }
}

void printDay(const struct DayPlan *day, FILE *fp)
{
    int s;
    int W = 44;

    /* Sorted copy so we can print slots in chronological order */
    struct ScheduleSlot sorted[MAX_SLOTS_PER_DAY];
    int sortedCount;
    int i, j;

    if (day == NULL) return;

    /* Copy slots into local array */
    sortedCount = day->slotCount;
    for (i = 0; i < sortedCount; i++)
        sorted[i] = day->slots[i];

    /* Insertion sort by startMin (small N, no need for qsort) */
    for (i = 1; i < sortedCount; i++)
    {
        struct ScheduleSlot key = sorted[i];
        j = i - 1;
        while (j >= 0 && sorted[j].startMin > key.startMin)
        {
            sorted[j + 1] = sorted[j];
            j--;
        }
        sorted[j + 1] = key;
    }

    printSeparator('=', W, fp);

    printf("     DAY %d SCHEDULE\n", day->dayNumber);
    printf("     Total study: %.1f hrs\n", day->totalStudyHours);
    if (fp)
    {
        fprintf(fp, "     DAY %d SCHEDULE\n", day->dayNumber);
        fprintf(fp, "     Total study: %.1f hrs\n", day->totalStudyHours);
    }

    printSeparator('=', W, fp);

    for (s = 0; s < sortedCount; s++)
    {
        const struct ScheduleSlot *sl = &sorted[s];
        char tStart[6], tEnd[6];
        minToTimeStr(sl->startMin, tStart);
        minToTimeStr(sl->endMin,   tEnd);

        if (sl->difficulty == 0)
        {
            printf("| %s - %s | %-22s |\n", tStart, tEnd, sl->activity);
            if (fp)
                fprintf(fp, "| %s - %s | %-22s |\n", tStart, tEnd, sl->activity);
        }
        else
        {
            printf("| %s - %s | [%s] %-14s |\n",
                   tStart, tEnd, diffLabel(sl->difficulty), sl->activity);
            if (fp)
                fprintf(fp, "| %s - %s | [%s] %-14s |\n",
                        tStart, tEnd, diffLabel(sl->difficulty), sl->activity);
        }

        printSeparator('-', W, fp);
    }

    if (rand() % 2 == 0)
    {
        const char *msg = funMessage();
        printf("  >> %s\n", msg);
        if (fp) fprintf(fp, "  >> %s\n", msg);
    }

    printSeparator('=', W, fp);
    printf("\n");
    if (fp) fputc('\n', fp);
}


static int placeStudyBlock(struct DayPlan *week,
                           int cursors[][3],
                           int dayIdx,
                           int bandIdx,
                           int diff,
                           const char *name,
                           int requestedMins)
{
    int bEnd     = bandEnd(diff);
    int cur      = cursors[dayIdx][bandIdx];
    int avail    = bEnd - cur;
    int study;
    float capLeft;
    int   capMins;
    char  blabel[80];
    int   choice;

    study = (requestedMins <= avail) ? requestedMins : avail;
    if (study <= 0) return 0;

    capLeft = MAX_NIGHT_STUDY_HRS - week[dayIdx].totalStudyHours;
    capMins = (int)(capLeft * 60.0f);
    if (capMins <= 0) return 0;
    if (study > capMins) study = capMins;
    if (study <= 0) return 0;

    if (!addSlot(&week[dayIdx], cur, study, name, diff)) return 0;
    cur += study;

    if (cur + BREAK_DURATION <= bEnd)
    {
        choice = rand() % 3;
        if      (choice == 0) snprintf(blabel, sizeof(blabel), "Break - Chai time!");
        else if (choice == 1) snprintf(blabel, sizeof(blabel), "Break - Stretch & rest eyes");
        else                  snprintf(blabel, sizeof(blabel), "Break - %s", funMessage());

        addSlot(&week[dayIdx], cur, BREAK_DURATION, blabel, 0);
        cur += BREAK_DURATION;
    }

    cursors[dayIdx][bandIdx] = cur;
    return 1;
}

static void buildWeekPlans(struct DayPlan week[MAX_DAYS])
{
    /* Loop variables and working variables — all declared at top */
    int  d, i, a, s;
    int  cursors[MAX_DAYS][3];
    int  assignDay;
    char revName[100];

    /* ── Initialise day plans ── */
    for (d = 0; d < MAX_DAYS; d++)
    {
        week[d].dayNumber       = d + 1;
        week[d].slotCount       = 0;
        week[d].totalStudyHours = 0.0f;
    }

    /* ── Initialise band cursors ── */
    for (d = 0; d < MAX_DAYS; d++)
    {
        cursors[d][0] = HARD_START_MIN;
        cursors[d][1] = MEDIUM_START_MIN;
        cursors[d][2] = EASY_START_MIN;
    }

    /* ── Place subjects ── */
    for (i = 0; i < subjectCount; i++)
    {
        int diff, band, placed, chosenDay, attempt;

        if (subjects[i].name[0] == '\0') continue;

        diff      = subjectDiff(i);
        band      = diff - 1;
        placed    = 0;
        chosenDay = -1;

        for (attempt = 0; attempt < MAX_DAYS; attempt++)
        {
            int n     = (i + attempt) % MAX_DAYS;
            int bEnd  = bandEnd(diff);
            int avail = bEnd - cursors[n][band];
            float capLeft;

            if (avail <= 0) continue;

            capLeft = MAX_NIGHT_STUDY_HRS - week[n].totalStudyHours;
            if (capLeft <= 0.0f) continue;

            chosenDay = n;
            break;
        }

        if (chosenDay == -1)
        {
            fprintf(stderr,
                    "  [Scheduler] No room for subject '%s' in any day – skipped.\n",
                    subjects[i].name);
            continue;
        }

        placed = placeStudyBlock(week, cursors, chosenDay, band,
                                 diff, subjects[i].name, 120);
        if (!placed)
        {
            fprintf(stderr,
                    "  [Scheduler] Could not place subject '%s' on day %d.\n",
                    subjects[i].name, chosenDay + 1);
        }
    }

    /* ── Place assignments ── */
    assignDay = 0;

    for (a = 0; a < assignmentCount; a++)
    {
        int totalMins, remaining;
        char activity[160];

        if (assignments[a].isCompleted) continue;

        if (assignments[a].timeRequired <= 0.0f)
        {
            fprintf(stderr,
                    "  [Scheduler] Assignment '%s' has non-positive timeRequired (%.2f) – skipped.\n",
                    assignments[a].name[0] ? assignments[a].name : "(unnamed)",
                    assignments[a].timeRequired);
            continue;
        }

        totalMins = (int)(assignments[a].timeRequired * 60.0f);
        remaining = totalMins;

        while (remaining > 0 && assignDay < MAX_DAYS)
        {
            int   bEnd    = NIGHT_END_MIN;
            int   cur     = cursors[assignDay][2];
            int   avail   = bEnd - cur;
            float capLeft = MAX_NIGHT_STUDY_HRS - week[assignDay].totalStudyHours;
            int   capMins = (int)(capLeft * 60.0f);
            int   usable, chunk;
            char  blabel[80];

            usable = (avail < capMins) ? avail : capMins;
            if (usable <= 0) { assignDay++; continue; }

            chunk = (remaining <= usable) ? remaining : usable;

            if (chunk < totalMins)
                snprintf(activity, sizeof(activity),
                         "Assignment: %s (Part, Due: %s)",
                         assignments[a].name[0]     ? assignments[a].name     : "Unnamed",
                         assignments[a].deadline[0] ? assignments[a].deadline : "?");
            else
                snprintf(activity, sizeof(activity),
                         "Assignment: %s (Due: %s)",
                         assignments[a].name[0]     ? assignments[a].name     : "Unnamed",
                         assignments[a].deadline[0] ? assignments[a].deadline : "?");

            addSlot(&week[assignDay], cur, chunk, activity, 2);
            cur += chunk;

            if (chunk >= 60 && cur + BREAK_DURATION <= bEnd)
            {
                snprintf(blabel, sizeof(blabel), "Break - %s", funMessage());
                addSlot(&week[assignDay], cur, BREAK_DURATION, blabel, 0);
                cur += BREAK_DURATION;
            }

            cursors[assignDay][2] = cur;
            remaining -= chunk;

            if (remaining > 0) assignDay++;
        }

        assignDay++;
        if (assignDay >= MAX_DAYS) assignDay = MAX_DAYS - 1;
    }

    /* ── Pick revision subject name ── */
    revName[0] = '\0';
    for (s = 0; s < subjectCount; s++)
    {
        if (strcmp(subjects[s].status, "Weak") == 0)
        {
            strncpy(revName, subjects[s].name, sizeof(revName) - 1);
            revName[sizeof(revName) - 1] = '\0';
            break;
        }
    }
    if (revName[0] == '\0')
        strncpy(revName, "General Revision", sizeof(revName) - 1);

    /* ── Fill unused band slots & wrap-up ── */
    for (d = 0; d < MAX_DAYS; d++)
    {
        /* Variables for wrap-up block — declared here at top of loop body */
        int wrapStart   = 1545;
        int wrapDur     = 15;
        int wrapEnd     = wrapStart + wrapDur;
        int latestCursor;
        char act[120];

        /* Hard band: add revision if untouched */
        if (cursors[d][0] == HARD_START_MIN &&
            week[d].totalStudyHours < MAX_NIGHT_STUDY_HRS)
        {
            int startAt = cursors[d][0];
            int bEnd    = MEDIUM_START_MIN;
            int dur     = 60;
            if (startAt + dur <= bEnd)
            {
                snprintf(act, sizeof(act), "Revision - %s", revName);
                addSlot(&week[d], startAt, dur, act, 1);
                cursors[d][0] += dur;
            }
        }

        /* Medium band: add revision / practice if untouched */
        if (cursors[d][1] == MEDIUM_START_MIN &&
            week[d].totalStudyHours < MAX_NIGHT_STUDY_HRS)
        {
            int startAt = cursors[d][1];
            int bEnd    = EASY_START_MIN;
            int dur     = 60;
            if (startAt + dur <= bEnd)
            {
                addSlot(&week[d], startAt, dur,
                        "Revision / Practice Problems", 2);
                cursors[d][1] += dur;
            }
        }

        /* Easy band: free slot if untouched */
        if (cursors[d][2] == EASY_START_MIN)
        {
            int startAt = cursors[d][2];
            int bEnd    = NIGHT_END_MIN;
            int dur     = 60;
            if (startAt + dur <= bEnd)
            {
                addSlot(&week[d], startAt, dur,
                        "Free Slot - Rest or Light Reading", 0);
                cursors[d][2] += dur;
            }
        }

        /* Wrap-up block */
        latestCursor = cursors[d][0];
        if (cursors[d][1] > latestCursor) latestCursor = cursors[d][1];
        if (cursors[d][2] > latestCursor) latestCursor = cursors[d][2];

        if (wrapStart >= latestCursor && wrapEnd <= NIGHT_END_MIN)
        {
            addSlot(&week[d], wrapStart, wrapDur,
                    "Wrap-Up: Set tomorrow's goals", 3);
        }
    }
}


void generateWeeklySchedule(void)
{
    struct DayPlan week[MAX_DAYS];
    int d;
    int W = 44;
    FILE *fp;

    srand((unsigned int)time(NULL));

    createTaskList();
    sortTasksByPriority();

    if (subjectCount == 0 && assignmentCount == 0)
    {
        printf("\n  [!] No subjects or assignments found.\n");
        printf("  [!] Add subjects (option 1) or assignments (option 7) first.\n");
        return;
    }

    buildWeekPlans(week);

    fp = fopen("studyplan.txt", "w");
    if (!fp)
        fprintf(stderr,
                "  [Warning] Could not open studyplan.txt – printing to console only.\n");

    printSeparator('*', W, fp);
    printf("*     7-DAY NIGHT STUDY SCHEDULE           *\n");
    printf("*     Night slots: 18:00 - 02:00           *\n");
    if (fp)
    {
        fprintf(fp, "*     7-DAY NIGHT STUDY SCHEDULE           *\n");
        fprintf(fp, "*     Night slots: 18:00 - 02:00           *\n");
    }
    printSeparator('*', W, fp);
    printf("\n");
    if (fp) fputc('\n', fp);

    for (d = 0; d < MAX_DAYS; d++)
        printDay(&week[d], fp);

    printf("  >> %s\n", funMessage());
    printf("  >> Stick to the plan — you've got this!\n\n");

    if (fp)
    {
        fprintf(fp, "  >> %s\n", funMessage());
        fprintf(fp, "  >> Stick to the plan — you've got this!\n\n");
        fclose(fp);
        printf("  [+] Schedule saved to studyplan.txt\n");
    }
}


void generateNightStudyPlan(void)
{
    int pending;
    int i;
    FILE *fp;

    srand((unsigned int)time(NULL));

    createTaskList();
    sortTasksByPriority();

    pending = -1;
    for (i = 0; i < assignmentCount; i++)
    {
        if (!assignments[i].isCompleted) { pending = i; break; }
    }

    printf("\n  ============================================\n");
    printf("       NIGHT STUDY PLAN -- Generated\n");
    printf("  ============================================\n");

    printf("  18:00 - 21:00  |  HIGH-PRIORITY STUDY\n");
    if (taskCount == 0)
        printf("                 |  (No tasks -- add subjects first)\n");
    else
        printf("                 |  %s\n", tasks[0].title);

    printf("  21:00 - 22:00  |  REVISION\n");
    if (taskCount < 2)
        printf("                 |  (Revisit Slot 1 topic)\n");
    else
        printf("                 |  %s\n", tasks[1].title);

    printf("  22:00 - 22:30  |  BREAK -- Rest, eat, recharge\n");

    printf("  22:30 - 00:30  |  ASSIGNMENT / STUDY\n");
    if (pending == -1)
        printf("                 |  (No pending assignments -- free study)\n");
    else
        printf("                 |  %s  (Due: %s, %.2f hrs)\n",
               assignments[pending].name[0]     ? assignments[pending].name     : "Unnamed",
               assignments[pending].deadline[0] ? assignments[pending].deadline : "?",
               assignments[pending].timeRequired);

    printf("  00:30 - 01:30  |  PRACTICE SESSION\n");
    printf("                 |  Solve problems / past papers.\n");

    printf("  01:30 - 02:00  |  WRAP-UP -- Note tomorrow's goals\n");

    printf("  ============================================\n");
    printf("  >> %s\n\n", funMessage());

    fp = fopen("night_plan.txt", "w");
    if (!fp)
    {
        fprintf(stderr, "  [Error] Could not save night_plan.txt\n");
        return;
    }

    fprintf(fp, "============================================\n");
    fprintf(fp, "     NIGHT STUDY PLAN -- Generated\n");
    fprintf(fp, "============================================\n");

    fprintf(fp, "18:00 - 21:00  |  HIGH-PRIORITY STUDY\n");
    if (taskCount == 0)
        fprintf(fp, "               |  (No tasks available)\n");
    else
        fprintf(fp, "               |  %s\n", tasks[0].title);

    fprintf(fp, "21:00 - 22:00  |  REVISION\n");
    if (taskCount < 2)
        fprintf(fp, "               |  (Revisit Slot 1 topic)\n");
    else
        fprintf(fp, "               |  %s\n", tasks[1].title);

    fprintf(fp, "22:00 - 22:30  |  BREAK -- Rest, eat, recharge\n");

    fprintf(fp, "22:30 - 00:30  |  ASSIGNMENT / STUDY\n");
    if (pending == -1)
        fprintf(fp, "               |  (No pending assignments)\n");
    else
        fprintf(fp, "               |  %s  (Due: %s, %.2f hrs)\n",
                assignments[pending].name[0]     ? assignments[pending].name     : "Unnamed",
                assignments[pending].deadline[0] ? assignments[pending].deadline : "?",
                assignments[pending].timeRequired);

    fprintf(fp, "00:30 - 01:30  |  PRACTICE SESSION\n");
    fprintf(fp, "               |  Solve problems / past papers.\n");
    fprintf(fp, "01:30 - 02:00  |  WRAP-UP -- Note tomorrow's goals\n");
    fprintf(fp, "============================================\n");

    fclose(fp);
    printf("  [+] Saved to night_plan.txt\n");
}