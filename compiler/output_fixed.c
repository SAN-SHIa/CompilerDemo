// Auto-generated C code - manually fixed

#include <stdio.h>
#include <string.h>

int main() {
    int x;
    float y, result;
    float t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    float t13, t14, t15, t19, t20, t21, t22, t23, t24;
    int t12, t16, t18, t25, t27, t28;
    char *t11, *t17, *t26;  // string temporaries

    x = 10;
    y = 5.50;
    t1 = x;
    t2 = y;
    t3 = t1 * t2;  // 10 * 5.5 = 55.0
    t4 = x;
    t5 = y;
    t6 = t4 - t5;  // 10 - 5.5 = 4.5
    t7 = t3 + t6;  // 55.0 + 4.5 = 59.5
    result = t7;   // result = 59.5
    t8 = result;
    t9 = (float)t8;
    t10 = t9 > 40.00;  // 59.5 > 40.0 = 1 (true)
    if (!t10) goto L1; // !1 = 0 (false), so this branch is NOT taken
    t11 = "Initial result is greater than 40.0";
    printf("%s\n", t11);  // This will be executed
    goto L2;
L1:
    t13 = result;
    t14 = (float)t13;
    t15 = t14 * 2.00;
    // FIXED: Don't convert to int
    result = t15;  // Keep as float
    t17 = "Doubled result";
    printf("%s\n", t17);
L2:
L3:
    t19 = result;   // result is 59.5
    t20 = (float)t19;
    t21 = t20 < 100.00;  // 59.5 < 100.0 = 1 (true)
    if (!t21) goto L4;  // !1 = 0 (false), so continue in loop
    t22 = result;
    t23 = (float)t22;
    t24 = t23 + 10.50;   // Add 10.5 each time
    // FIXED: Don't convert to int
    result = t24;  // Keep as float
    goto L3;
L4:
    t26 = "Final result: %f\n";
    t27 = result;
    printf(t26, t27);
    return 0;
}
