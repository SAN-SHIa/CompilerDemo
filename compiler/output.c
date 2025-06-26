// Auto-generated C code

// Auto-generated C code

#include <stdio.h>
#include <string.h>

int main() {
    int x;
    float y, result;
    float t3, t4, t5, t7, t8, t9, t10, t11, t12, t15, t16, t18, t20, t21, t22, t23, t25;
    int t1, t2, t6, t14, t19, t26;
    char *t13, *t17, *t24;  // string temporaries

    t1 = 10;
    x = t1;
    y = 5.50;
    t2 = x;
    t3 = y;
    t4 = (float)t2;
    t5 = t4 * t3;
    t6 = x;
    t7 = y;
    t8 = (float)t6;
    t9 = t8 - t7;
    t10 = t5 + t9;
    result = t10;
    t11 = result;
    t12 = t11 > 40.00;
    if (!t12) goto L1;
    t13 = "Initial result is greater than 40.0\n";
    printf(t13);
    goto L2;
L1:
    t15 = result;
    t16 = t15 * 2.00;
    result = t16;
    t17 = "Doubled result: %f\n";
    t18 = result;
    printf(t17, t18);
L2:
L3:
    t20 = result;
    t21 = t20 < 100.00;
    if (!t21) goto L4;
    t22 = result;
    t23 = t22 + 10.50;
    result = t23;
    goto L3;
L4:
    t24 = "Final result: %f\n";
    t25 = result;
    printf(t24, t25);
    return 0;
}

