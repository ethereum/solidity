contract C {
    struct S {
        mapping(uint => uint) c;
    }
    S public constant e = 0x1212121212121212121212121212121212121212;
}
// ----
// DeclarationError 9259: (71-135): Constants of non-value type not yet implemented.
