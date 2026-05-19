contract C {
    int[uint(keccak256(bytes.concat("ABCD")))] arr;
}
// ----
// TypeError 5462: (21-58): Invalid array length, expected integer literal or constant expression.
