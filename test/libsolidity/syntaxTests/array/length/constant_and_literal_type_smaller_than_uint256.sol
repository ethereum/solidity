contract C {
    uint8 constant LEN = 254;
    uint[LEN + 1] array1;
    uint[LEN + 256] array2;
    uint[LEN * 2] array3; // Error, outside of constant and literal range
}
// ----
// TypeError 2643: (106-113): Arithmetic error when computing constant value.
