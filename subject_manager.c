#include <stdio.h>
#include <string.h>
#include <stdlib.h>   // ADDED
#include "subject_manager.h"

struct Subject subjects[MAX_SUBJECTS];
int subjectCount = 0;




int checkCancel(char *input)
{
    input[strcspn(input, "\n")] = 0;
    if(strcmp(input, "cancel") == 0)
    {
        printf("Operation cancelled.\n");
        return 1;
    }
    return 0;
}

void calculateSubjectScore(struct Subject *s)
{
    s->finalScore = (0.4 * s->quizPercent) +
                    (0.4 * s->minorPercent) +
                    (0.2 * (s->confidence * 20));
}

void assignStatus(struct Subject *s)
{
    if(s->finalScore < 30)
        strcpy(s->status, "Weak");
    else if(s->finalScore <= 50)
        strcpy(s->status, "Average");
    else if(s->finalScore <= 75)
        strcpy(s->status, "Moderate");
    else
        strcpy(s->status, "Strong");
}

void addSubject()
{
    if(subjectCount >= MAX_SUBJECTS)
    {
        printf("Subject limit reached!\n");
        return;
    }

    struct Subject s;
    char input[100];

    getchar();

    printf("Enter subject name (or 'cancel'): ");
    fgets(input, sizeof(input), stdin);
    if(checkCancel(input)) return;
    strcpy(s.name, input);

    float obtained, total;

    printf("Enter quiz marks obtained (or 'cancel'): ");
    scanf("%s", input);
    if(strcmp(input, "cancel") == 0) return;
    if(!isValidFloat(input)) { printf("Invalid input!\n"); return; } // ADDED
    obtained = atof(input);

    printf("Enter quiz total marks (or 'cancel'): ");
    scanf("%s", input);
    if(strcmp(input, "cancel") == 0) return;
    if(!isValidFloat(input)) { printf("Invalid input!\n"); return; } // ADDED
    total = atof(input);

    s.quizPercent = (total > 0) ? (obtained / total) * 100 : 0;

    printf("Enter minor marks obtained (or 'cancel'): ");
    scanf("%s", input);
    if(strcmp(input, "cancel") == 0) return;
    if(!isValidFloat(input)) { printf("Invalid input!\n"); return; } // ADDED
    obtained = atof(input);

    printf("Enter minor total marks (or 'cancel'): ");
    scanf("%s", input);
    if(strcmp(input, "cancel") == 0) return;
    if(!isValidFloat(input)) { printf("Invalid input!\n"); return; } // ADDED
    total = atof(input);

    s.minorPercent = (total > 0) ? (obtained / total) * 100 : 0;

    printf("Enter confidence level (1-5) (or 'cancel'): ");
    scanf("%s", input);
    if(strcmp(input, "cancel") == 0) return;
    if(!isValidInt(input)) { printf("Invalid input!\n"); return; } // ADDED
    s.confidence = atoi(input);

    calculateSubjectScore(&s);
    assignStatus(&s);

    subjects[subjectCount] = s;
    subjectCount++;

    saveSubjectsToFile();

    printf("Subject added successfully!\n");
}


void viewSubjects()
{
    if(subjectCount == 0)
    {
        printf("No subjects available.\n");
        return;
    }

    printf("\nSubjects:\n");

    for(int i = 0; i < subjectCount; i++)
    {
        printf("\nSubject: %s\n", subjects[i].name);
        printf("Quiz Percentage: %.2f\n", subjects[i].quizPercent);
        printf("Minor Percentage: %.2f\n", subjects[i].minorPercent);
        printf("Confidence: %d\n", subjects[i].confidence);
        printf("Final Score: %.2f\n", subjects[i].finalScore);
        printf("Status: %s\n", subjects[i].status);
    }
}

void updateSelfAssessment()
{
    char name[100];

    getchar();
    printf("Enter subject name (or 'cancel'): ");
    fgets(name, sizeof(name), stdin);
    if(checkCancel(name)) return;

    for(int i = 0; i < subjectCount; i++)
    {
        if(strcmp(subjects[i].name, name) == 0)
        {
            char input[50];
            printf("Enter new confidence (1-5) (or 'cancel'): ");
            scanf("%s", input);
            if(strcmp(input, "cancel") == 0) return;

            subjects[i].confidence = atoi(input);

            calculateSubjectScore(&subjects[i]);
            assignStatus(&subjects[i]);

            saveSubjectsToFile();

            printf("Updated successfully!\n");
            return;
        }
    }

    printf("Subject not found.\n");
}

void saveSubjectsToFile()
{
    FILE *fp = fopen("subjects.txt", "w");  

    for(int i = 0; i < subjectCount; i++)
    {
        fprintf(fp,"%s|%f|%f|%d|%f|%s\n",
            subjects[i].name,
            subjects[i].quizPercent,
            subjects[i].minorPercent,
            subjects[i].confidence,
            subjects[i].finalScore,
            subjects[i].status);
    }

    fclose(fp);
}

void deleteSubject()
{
    char name[100];
    int found = 0;

    getchar();

    printf("Enter subject name to delete (or 'cancel'): ");
    fgets(name,sizeof(name),stdin);
    if(checkCancel(name)) return;

    for(int i=0;i<subjectCount;i++)
    {
        if(strcmp(subjects[i].name,name)==0)
        {
            found = 1;

            for(int j=i;j<subjectCount-1;j++)
                subjects[j] = subjects[j+1];

            subjectCount--;
            saveSubjectsToFile();

            printf("Subject deleted successfully.\n");
            break;
        }
    }

    if(!found)
        printf("Subject not found.\n");
}

void editSubject()
{
    char name[100];
    int found = 0;

    getchar();

    printf("Enter subject name to edit (or 'cancel'): ");
    fgets(name,sizeof(name),stdin);
    if(checkCancel(name)) return;

    for(int i=0;i<subjectCount;i++)
    {
        if(strcmp(subjects[i].name,name)==0)
        {
            found = 1;

            float obtained,total;
            char input[50];

            printf("Enter new quiz marks obtained (or 'cancel'): ");
            scanf("%s", input);
            if(strcmp(input, "cancel") == 0) return;
            obtained = atof(input);

            printf("Enter quiz total marks (or 'cancel'): ");
            scanf("%s", input);
            if(strcmp(input, "cancel") == 0) return;
            total = atof(input);

            subjects[i].quizPercent = (total > 0) ? (obtained/total)*100 : 0;

            printf("Enter new minor marks obtained (or 'cancel'): ");
            scanf("%s", input);
            if(strcmp(input, "cancel") == 0) return;
            obtained = atof(input);

            printf("Enter minor total marks (or 'cancel'): ");
            scanf("%s", input);
            if(strcmp(input, "cancel") == 0) return;
            total = atof(input);

            subjects[i].minorPercent = (total > 0) ? (obtained/total)*100 : 0;

            printf("Enter new confidence (1-5) (or 'cancel'): ");
            scanf("%s", input);
            if(strcmp(input, "cancel") == 0) return;
            subjects[i].confidence = atoi(input);

            calculateSubjectScore(&subjects[i]);
            assignStatus(&subjects[i]);

            saveSubjectsToFile();

            printf("Subject updated successfully.\n");
            break;
        }
    }

    if(!found)
        printf("Subject not found.\n");
}

void loadSubjectsFromFile()
{
    FILE *fp = fopen("subjects.txt", "r");

    if(fp == NULL)
        return;

    while(subjectCount < MAX_SUBJECTS &&
        fscanf(fp,"%[^|]|%f|%f|%d|%f|%[^\n]\n",
        subjects[subjectCount].name,
        &subjects[subjectCount].quizPercent,
        &subjects[subjectCount].minorPercent,
        &subjects[subjectCount].confidence,
        &subjects[subjectCount].finalScore,
        subjects[subjectCount].status) != EOF)
    {
        subjectCount++;
    }

    fclose(fp);
}

void resetSubjects()
{
    char confirm;

    printf("Are you sure you want to reset all subjects? (y/n): ");
    scanf(" %c", &confirm);

    if(confirm == 'y' || confirm == 'Y')
    {
        subjectCount = 0;
        FILE *fp = fopen("subjects.txt", "w");
        if(fp) fclose(fp);
        printf("Reset successful!\n");
    }
    else
    {
        printf("Reset cancelled.\n");
    }
}