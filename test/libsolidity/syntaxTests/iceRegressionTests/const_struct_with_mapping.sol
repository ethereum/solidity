contract C {
    struct S {
        mapping(uint => uint) c;
    }
    S public constant e = 0x1212121212121212121212121212121212121212;
}
// ----
// DeclarationError 3530: (71-135): The type contains a (nested) mapping and therefore cannot be a constant.
