contract C {
    bytes32 constant X = "00000000000000000000000000000000000000000000000000000000000000001";
    int[uint(X)] array;
}
// ----
// TypeError 5462: (115-122): Invalid array length, expected integer literal or constant expression.
