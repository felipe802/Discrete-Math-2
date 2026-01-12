#include <stdio.h>
#include <stdlib.h>

typedef struct pt{
    int x, y;
}Pt;

int domina(Pt* p, Pt* q){
    return (q->x >= p->x && q->y >= p->y);
}

void max(int n, Pt* pts[]) {
    for (int i = 0; i < n; i++) {
        int dominated = 0; //false
        for (int j = 0; j < n; j++) {
            if (i != j && domina(pts[i], pts[j])) {
                dominated = 1; //true
                break; //let's next, this was died
            }
        }
        if (!dominated) { //nobody wons pts[i], next round
            printf("max: (%d, %d)\n", pts[i]->x, pts[i]->y);
        }
    }
}

int main() {
    Pt* pts[3];
    for (int i = 0; i < 3; i++) pts[i] = malloc(sizeof(Pt));

    pts[0]->x = 0; pts[0]->y = 4;
    pts[1]->x = 2; pts[1]->y = 5;
    pts[2]->x = 3; pts[2]->y = 3;

    max(3, pts);

    for (int i = 0; i < 3; i++) free(pts[i]);

    return 0;
}