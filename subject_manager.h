#ifndef SUBJECT_MANAGER_H // ifndef to prevent multiple inclusions of this header file
#define SUBJECT_MANAGER_H // #define SUBJECT_MANAGER_H is used to define the header guard 

#define MAX_SUBJECTS 20 // avoid hardcoding the maximum number of subjects, making it easier to change in the future if needed

struct Subject
{
    char name[100];
    float quizPercent;
    float minorPercent;
    int confidence;
    float finalScore;
    char status[20];
};

extern struct Subject subjects[MAX_SUBJECTS]; // extern is used to declare the subjects array, which will be defined in the corresponding .cpp file
extern int subjectCount;

void addSubject();
void viewSubjects();
void updateSelfAssessment();
void calculateSubjectScore(struct Subject *s);
void assignStatus(struct Subject *s);
void deleteSubject();
void editSubject();
void saveSubjectsToFile();
void loadSubjectsFromFile();
void resetSubjects();

#endif // end of the header guard