// students.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Student {
    char   name[50];
    int    age;
    double gpa;
};

static int cmp_gpa_desc(const void *a, const void *b) {
    const struct Student *x = a, *y = b;
    return (x->gpa < y->gpa) - (x->gpa > y->gpa);
}

int main(void) {
    int n;
    printf("So sinh vien: ");
    if (scanf("%d", &n) != 1 || n <= 0) return 1;

    struct Student *s = calloc((size_t)n, sizeof *s);
    if (!s) return 1;
    for (int i = 0; i < n; i++) {
        printf("Ten tuoi gpa #%d: ", i + 1);
        if (scanf("%49s %d %lf", s[i].name, &s[i].age, &s[i].gpa) != 3) { free(s); return 1; }
    }
    qsort(s, (size_t)n, sizeof *s, cmp_gpa_desc);

    printf("\n%-20s %4s %5s\n", "Ten", "Tuoi", "GPA");
    for (int i = 0; i < n; i++) printf("%-20s %4d %5.2f\n", s[i].name, s[i].age, s[i].gpa);
    free(s);
    return 0;
}
