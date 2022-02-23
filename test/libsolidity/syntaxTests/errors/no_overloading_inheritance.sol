contract A {
    error Err(uint);
}
contract B is A {
    error Err(bytes32);
}
// ----
// DeclarationError 9097: (58-77): Identifier already declared.
