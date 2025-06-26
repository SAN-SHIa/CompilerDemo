int main() {
    int x = 10;
    float y = 5.5;
    float result;
    
    result = (x * y) + (x - y);
    
    if (result > 40.0) {
        printf("Initial result is greater than 40.0\n");
    } else {
        result = result * 2.0;
        printf("Doubled result: %f\n", result);
    }
    
    while (result < 100.0) {
        result = result + 10.5;
    }
    
    printf("Final result: %f\n", result);
    return 0;
}
