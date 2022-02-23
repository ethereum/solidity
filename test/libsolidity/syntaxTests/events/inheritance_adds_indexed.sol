contract A {
    event X(uint);
}
contract B is A {
    event X(uint indexed);
}
// ----
// DeclarationError 5883: (56-78): Event with same name and parameter types defined twice.
