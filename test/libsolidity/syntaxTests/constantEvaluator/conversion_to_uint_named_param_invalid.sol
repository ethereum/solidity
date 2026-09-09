contract C {
    int[uint({val: 77})] array;
}
// ----
// TypeError 5462: (21-36): Invalid array length, expected integer literal or constant expression.
