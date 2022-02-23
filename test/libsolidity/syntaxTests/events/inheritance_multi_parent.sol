contract A {
    event X(uint, uint indexed);
}
contract B {
    event X(uint, uint);
}
contract C is A, B {
}
// ----
// DeclarationError 5883: (65-85): Event with same name and parameter types defined twice.
