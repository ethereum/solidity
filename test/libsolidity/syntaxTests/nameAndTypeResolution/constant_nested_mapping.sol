contract C {
    struct S {
        mapping(uint => uint) x;
    }
    S public constant c;
}
// ----
// DeclarationError 3530: (71-90): The type contains a (nested) mapping and therefore cannot be a constant.
