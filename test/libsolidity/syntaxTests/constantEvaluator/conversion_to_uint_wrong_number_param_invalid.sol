contract C {
    int[uint()] array1;
    int[uint(1, 2)] array2;
}
// ----
// TypeError 5462: (21-27): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (45-55): Invalid array length, expected integer literal or constant expression.
