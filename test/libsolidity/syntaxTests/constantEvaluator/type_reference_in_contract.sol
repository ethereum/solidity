contract C {
    int[L] constant L = 6;
}
// ----
// TypeError 5462: (21-22): Invalid array length, expected integer literal or constant expression.
// TypeError 9259: (17-38): Only constants of value type and byte array type are implemented.
