// csv_scores.c
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    FILE *f = fopen(argc > 1 ? argv[1] : "scores.csv", "r");
    if (!f) { perror("scores.csv"); return 1; }

    char line[256], best_name[64] = "";
    double sum = 0, best = -1;
    int n = 0, lineno = 0, bad = 0;

    while (fgets(line, sizeof line, f)) {
        lineno++;
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0') continue;                                // bỏ dòng trống

        char *comma = strrchr(line, ',');                             // dấu phẩy cuối: tên có thể chứa dấu phẩy
        if (!comma) { fprintf(stderr, "dong %d: thieu dau phay\n", lineno); bad++; continue; }
        *comma = '\0';

        char *end;
        errno = 0;
        double score = strtod(comma + 1, &end);
        if (end == comma + 1 || *end != '\0' || errno == ERANGE || score < 0 || score > 10) {
            fprintf(stderr, "dong %d: diem khong hop le\n", lineno);
            bad++;
            continue;
        }
        sum += score;
        n++;
        if (score > best) { best = score; snprintf(best_name, sizeof best_name, "%s", line); }
    }
    fclose(f);

    if (n == 0) { fprintf(stderr, "khong co du lieu hop le\n"); return 1; }
    printf("%d hop le (%d loi), diem trung binh = %.2f, cao nhat: %s (%.1f)\n", n, bad, sum / n, best_name, best);
    return 0;
}
