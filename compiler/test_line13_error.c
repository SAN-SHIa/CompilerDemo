int main() {
    int x = 10;
    float y = 5.5;
    float z = 3.14;
    float result;
    
    result = x + y + z;
    
    if (result > 15.0) {
        printf("Result is large\n");
    }
    
    // 第13行故意缺少分号
    printf("The result is: %f\n", result);
    
    return 0;
}
