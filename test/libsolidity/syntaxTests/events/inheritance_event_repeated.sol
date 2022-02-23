contract A {
    event X(uint);
}
contract B is A {
    event X(uint);
}
// ----
// DeclarationError 5883: (56-70): Event with same name and parameter types defined twice.
