contract C {
    struct S {
        mapping(uint => uint) x;
    }
    S public constant c;
}
// ----
// TypeError 9259: (71-90): Only constants of value type and byte array type are implemented.
