contract C {
    uint immutable z = 2;
    uint immutable x = z = y = 3;
    uint immutable y = 5;
}
