contract C {
    struct S {
        mapping(uint => uint) c;
    }
    S public constant e = 0x1212121212121212121212121212121212121212;
}
// ----
// TypeError 9259: (71-135): Only constants of value type and byte array type are implemented.
