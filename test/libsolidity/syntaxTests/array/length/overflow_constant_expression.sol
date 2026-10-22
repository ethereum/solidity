uint constant LEN = 2**255;
uint constant MUL = 2;
contract C {
    uint[LEN * MUL] array;
}
// ----
// TypeError 2643: (73-82): Arithmetic error when computing constant value.
