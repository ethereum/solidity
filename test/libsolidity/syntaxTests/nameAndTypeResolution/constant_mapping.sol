contract C {
    // This should probably have a better error message at some point.
    // Constant mappings should not be possible in general.
    mapping(uint => uint) constant x;
}
// ----
// DeclarationError 9259: (148-180): Constants of non-value type not yet implemented.
