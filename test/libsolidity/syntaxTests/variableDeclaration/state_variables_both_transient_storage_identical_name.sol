contract C {
    uint public x;
    uint public transient x;
}
// ====
// EVMVersion: >=cancun
// ----
// DeclarationError 2333: (36-59): Identifier already declared.
