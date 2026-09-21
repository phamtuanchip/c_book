// guess.c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void) {
    srand((unsigned)time(NULL));
    int secret = rand() % 100 + 1;                       // 1..100 (hơi lệch nhẹ nhưng đủ cho trò chơi)
    int guess, tries = 0;

    printf("Toi nghi ra mot so tu 1 den 100. Doan di!\n");
    do {
        printf("Doan: ");
        if (scanf("%d", &guess) != 1) {                  // chữ hoặc EOF
            int c;
            while ((c = getchar()) != '\n' && c != EOF) { }
            if (c == EOF) return 1;
            printf("Hay nhap mot so.\n");
            guess = 0;
            continue;                                    // trong do-while: nhảy tới kiểm tra điều kiện
        }
        tries++;
        if (guess < secret)      printf("Lon hon!\n");
        else if (guess > secret) printf("Nho hon!\n");
    } while (guess != secret);

    printf("Dung roi sau %d lan doan.\n", tries);
    return 0;
}
