
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------------------------------------------------
   CONSTANTS
--------------------------------------------------- */
#define SUBJECTS 3
#define FILE_NAME "students.csv"

const char *SUBJECT_NAMES[SUBJECTS] = {"Maths", "Science", "English"};

typedef struct
{
    int roll;
    char name[50];
    char father[50];
    float marks[SUBJECTS];
    float attendance;
    float base_fee;
} Student;

typedef struct
{
    Student *data;
    int count;
    int capacity;
} Pipeline;

/* ─── pipeline helpers ─────────────────────────── */
void pipeline_init(Pipeline *p)
{
    p->capacity = 4;
    p->count = 0;
    p->data = malloc(p->capacity * sizeof(Student));
    if (!p->data)
    {
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }
}

void pipeline_free(Pipeline *p)
{
    free(p->data);
    p->data = NULL;
    p->count = 0;
    p->capacity = 0;
}

void pipeline_push(Pipeline *p, Student s)
{
    if (p->count == p->capacity)
    {
        p->capacity *= 2;
        p->data = realloc(p->data, p->capacity * sizeof(Student));
        if (!p->data)
        {
            fprintf(stderr, "realloc failed\n");
            exit(1);
        }
    }
    p->data[p->count++] = s;
}

float overall(const Student *s)
{
    float sum = 0;
    for (int i = 0; i < SUBJECTS; i++)
        sum += s->marks[i];
    return sum / SUBJECTS;
}

float scholarship(float pct, float attendance, float base_fee)
{
    float rate = 0.0f;
    if (pct >= 90)
        rate = 0.75f;
    else if (pct >= 75)
        rate = 0.50f;
    else if (pct >= 60)
        rate = 0.25f;
    if (attendance >= 90 && rate > 0)
        rate += 0.05f;
    if (rate > 1.0f)
        rate = 1.0f;
    return base_fee * rate;
}

const char *recommend(float pct)
{
    if (pct >= 85)
        return "Engineering / Science College";
    else if (pct >= 70)
        return "Commerce / Mid-tier College";
    else if (pct >= 50)
        return "Arts / General College";
    else
        return "ITI / Polytechnic";
}

const char *eligible(float pct, float attendance)
{
    return (pct >= 33 && attendance >= 75) ? "YES" : "NO";
}

int cmp_roll(const void *a, const void *b)
{
    return ((Student *)a)->roll - ((Student *)b)->roll;
}

void pipeline_sort(Pipeline *p)
{
    qsort(p->data, p->count, sizeof(Student), cmp_roll);
}

Student *binary_search(Pipeline *p, int roll)
{
    int lo = 0, hi = p->count - 1;
    while (lo <= hi)
    {
        int mid = (lo + hi) / 2;
        if (p->data[mid].roll == roll)
            return &p->data[mid];
        else if (p->data[mid].roll < roll)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return NULL; /* not found */
}

void save_to_file(const Pipeline *p)
{
    FILE *f = fopen(FILE_NAME, "w");
    if (!f)
    {
        printf("Could not open file for saving.\n");
        return;
    }

    for (int i = 0; i < p->count; i++)
    {
        const Student *s = &p->data[i];
        fprintf(f, "%d|%s|%s|%.2f|%.2f|%.2f|%.2f|%.2f\n",
                s->roll, s->name, s->father,
                s->marks[0], s->marks[1], s->marks[2],
                s->attendance, s->base_fee);
    }
    fclose(f);
    printf("[Saved] %d student(s) written to %s\n", p->count, FILE_NAME);
}

void load_from_file(Pipeline *p)
{
    FILE *f = fopen(FILE_NAME, "r");
    if (!f)
        return; /* first run — no file yet, that's fine */

    Student s;
    while (fscanf(f, "%d|%49[^|]|%49[^|]|%f|%f|%f|%f|%f\n",
                  &s.roll, s.name, s.father,
                  &s.marks[0], &s.marks[1], &s.marks[2],
                  &s.attendance, &s.base_fee) == 8)
    {
        pipeline_push(p, s);
    }
    fclose(f);
    printf("[Loaded] %d student(s) from %s\n", p->count, FILE_NAME);
}

/* get a float within [min, max] — loops until valid */
float get_float(const char *prompt, float min, float max)
{
    float val;
    while (1)
    {
        printf("%s (%.0f–%.0f): ", prompt, min, max);
        if (scanf("%f", &val) == 1 && val >= min && val <= max)
            return val;
        printf("  Invalid. Enter a number between %.0f and %.0f.\n", min, max);
        while (getchar() != '\n')
            ; /* flush bad input */
    }
}

float get_positive(const char *prompt)
{
    float val;
    while (1)
    {
        printf("%s: ", prompt);
        if (scanf("%f", &val) == 1 && val >= 0)
            return val;
        printf("  Invalid. Enter a positive number.\n");
        while (getchar() != '\n')
            ;
    }
}

/* get an integer */
int get_int(const char *prompt)
{
    int val;
    while (1)
    {
        printf("%s: ", prompt);
        if (scanf("%d", &val) == 1)
            return val;
        printf("  Invalid. Enter a whole number.\n");
        while (getchar() != '\n')
            ;
    }
}

void print_report(const Student *s)
{
    float pct = overall(s);
    float schol = scholarship(pct, s->attendance, s->base_fee);

    printf("\n+---------------------------------------+\n");
    printf("| Roll: %-5d  %-24s|\n", s->roll, s->name);
    printf("| Father: %-30s|\n", s->father);
    printf("+---------------------------------------+\n");
    for (int i = 0; i < SUBJECTS; i++)
        printf("|  %-8s : %5.1f / 100   → %5.2f%%    |\n",
               SUBJECT_NAMES[i], s->marks[i], s->marks[i]);
    printf("|  Overall  :             → %5.2f%%    |\n", pct);
    printf("+---------------------------------------+\n");
    printf("|  Attendance   : %5.1f%%               |\n", s->attendance);
    printf("|  Exam Eligible: %-3s                  |\n", eligible(pct, s->attendance));
    printf("+---------------------------------------+\n");
    printf("|  Base Fee     : Rs %-10.2f        |\n", s->base_fee);
    printf("|  Scholarship  : Rs %-10.2f        |\n", schol);
    printf("|  Net Due      : Rs %-10.2f        |\n", s->base_fee - schol);
    printf("+---------------------------------------+\n");
    printf("|  College : %-27s|\n", recommend(pct));
    printf("+---------------------------------------+\n");
}

void action_add(Pipeline *p)
{
    Student s;
    memset(&s, 0, sizeof(s));

    printf("\n--- ADD STUDENT ---\n");
    s.roll = get_int("Roll No");

    /* check duplicate */
    pipeline_sort(p);
    if (binary_search(p, s.roll))
    {
        printf("Roll %d already exists.\n", s.roll);
        return;
    }

    printf("Name        : ");
    scanf("%49s", s.name);
    printf("Father Name : ");
    scanf("%49s", s.father);

    for (int i = 0; i < SUBJECTS; i++)
        s.marks[i] = get_float(SUBJECT_NAMES[i], 0, 100);

    s.attendance = get_float("Attendance %", 0, 100);
    s.base_fee = get_positive("Base Fee (Rs)");

    pipeline_push(p, s);
    pipeline_sort(p); /* keep sorted for binary search */
    save_to_file(p);
    printf("Student added.\n");
    print_report(&p->data[p->count - 1]);
}

void action_view_all(const Pipeline *p)
{
    if (p->count == 0)
    {
        printf("No students yet.\n");
        return;
    }
    printf("\n--- ALL STUDENTS (%d) ---\n", p->count);
    for (int i = 0; i < p->count; i++)
        print_report(&p->data[i]);
}

void action_search(Pipeline *p)
{
    int roll = get_int("\nRoll No to search");
    pipeline_sort(p);
    Student *s = binary_search(p, roll);
    if (s)
        print_report(s);
    else
        printf("Roll %d not found.\n", roll);
}

void action_revaluate(Pipeline *p)
{
    printf("\n--- REVALUATION PORTAL ---\n");
    int roll = get_int("Roll No");
    pipeline_sort(p);
    Student *s = binary_search(p, roll);
    if (!s)
    {
        printf("Roll %d not found.\n", roll);
        return;
    }

    printf("Select subject:\n");
    for (int i = 0; i < SUBJECTS; i++)
        printf("  %d. %s (current: %.1f)\n", i, SUBJECT_NAMES[i], s->marks[i]);

    int idx = get_int("Subject index");
    if (idx < 0 || idx >= SUBJECTS)
    {
        printf("Invalid index.\n");
        return;
    }

    float new_marks = get_float("New marks", 0, 100);
    printf("[REVAL] %s | %s: %.1f → %.1f\n",
           s->name, SUBJECT_NAMES[idx], s->marks[idx], new_marks);
    s->marks[idx] = new_marks;

    save_to_file(p);
    printf("\n[Updated Report]\n");
    print_report(s);
}

/* Delete a student */
void action_delete(Pipeline *p)
{
    int roll = get_int("\nRoll No to delete");
    pipeline_sort(p);
    Student *s = binary_search(p, roll);
    if (!s)
    {
        printf("Roll %d not found.\n", roll);
        return;
    }

    /* shift array left — remove the gap */
    int idx = (int)(s - p->data);
    for (int i = idx; i < p->count - 1; i++)
        p->data[i] = p->data[i + 1];
    p->count--;

    save_to_file(p);
    printf("Roll %d deleted.\n", roll);
}

int main(void)
{
    Pipeline p;
    pipeline_init(&p);
    load_from_file(&p);
    pipeline_sort(&p);

    int choice;
    do
    {
        printf("\n+------------------------------+\n");
        printf("|   STUDENT MANAGEMENT SYSTEM  |\n");
        printf("+------------------------------+\n");
        printf("|  1. Add Student              |\n");
        printf("|  2. View All                 |\n");
        printf("|  3. Search by Roll No        |\n");
        printf("|  4. Revaluation Portal       |\n");
        printf("|  5. Delete Student           |\n");
        printf("|  0. Exit                     |\n");
        printf("+------------------------------+\n");

        choice = get_int("Choice");

        switch (choice)
        {
        case 1:
            action_add(&p);
            break;
        case 2:
            action_view_all(&p);
            break;
        case 3:
            action_search(&p);
            break;
        case 4:
            action_revaluate(&p);
            break;
        case 5:
            action_delete(&p);
            break;
        case 0:
            printf("Goodbye.\n");
            break;
        default:
            printf("Invalid option.\n");
        }
    } while (choice != 0);

    pipeline_free(&p); /* free heap memory before exit */
    return 0;
}