// Auto-generated C code

// Auto-generated C code

#include <stdio.h>
#include <string.h>

int main() {
    int x;
    float y, result;
    float t2, t3, t4, t6, t7, t8, t9, t10, t11, t14, t15, t17, t19, t20, t21, t22, t24;
    int t1, t5, t13, t18, t25;
    char *t12, *t16, *t23;  // string temporaries

    x = 10;
    y = 5.50;
    t1 = x;
    t2 = y;
    t3 = (float)t1;
    t4 = t3 * t2;
    t5 = x;
    t6 = y;
    t7 = (float)t5;
    t8 = t7 - t6;
    t9 = t4 + t8;
    result = t9;
    t10 = result;
    t11 = t10 > 40.00;
    if (!t11) goto L1;
    t12 = "Initial result is greater than 40.0\n";
    printf(t12);
    goto L2;
L1:
    t14 = result;
    t15 = t14 * 2.00;
    result = t15;
    t16 = "Doubled result: %f\n";
    t17 = result;
    printf(t16, t17);
L2:
L3:
    t19 = result;
    t20 = t19 < 100.00;
    if (!t20) goto L4;
    t21 = result;
    t22 = t21 + 10.50;
    result = t22;
    goto L3;
L4:
    t23 = "Final result: %f\n";
    t24 = result;
    printf(t23, t24);
    return 0;
}

