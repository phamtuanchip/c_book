// write_text.c
#include <stdio.h>

int main(void) {
    FILE *f = fopen("scores.txt", "w");
    if (!f) { perror("fopen"); return 1; }

    const char *names[] = {"An", "Binh", "Cuong"};
    int scores[] = {8, 9, 7};
    for (int i = 0; i < 3; i++) {
        if (fprintf(f, "%s,%d\n", names[i], scores[i]) < 0) {   // fprintf trả số âm khi lỗi
            perror("fprintf");
            fclose(f);
            return 1;
        }
    }
    return fclose(f) == 0 ? 0 : 1;
}
