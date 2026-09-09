uint constant START = 10;
uint constant END = 2;
contract C {
    uint[END - START] array;
}
// ----
// TypeError 2643: (71-82): Arithmetic error when computing constant value.
