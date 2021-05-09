contract C {
    struct S {
        mapping(uint => uint) x;
    }
    S public constant c;
}
// ----
// DeclarationError 9259: (71-90): Constants of non-value type not yet implemented.
