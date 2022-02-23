contract C {
    struct S { uint x; uint[] y; }
    S constant x = S(5, new uint[](4));
}
// ----
// TypeError 9259: (52-86): Only constants of value type and byte array type are implemented.
