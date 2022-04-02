contract A {
    event X(uint, uint indexed);
}
contract B is A {
    event X(uint, uint);
}
// ----
// DeclarationError 5883: (70-90='event X(uint, uint);'): Event with same name and parameter types defined twice.
