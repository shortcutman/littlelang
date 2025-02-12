int64 counter;
counter = 1;
while (counter < 100) {
    int64 mod3;
    mod3 = counter % 3;

    int64 mod5;
    mod5 = counter % 5;
    if (mod3 == 0) {
        if (mod5 == 0) {
            printf("FizzBuzz");
        } else {
            printf("Fizz");
        }
    } else if (mod5 == 0) {
        printf("Buzz");
    } else {
        printf("%i", counter);
    }
    counter = counter + 1;
    puts("");
}