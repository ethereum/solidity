contract A {
    event X();
}
contract B is A {
    event X() anonymous;
}
// ----
// DeclarationError 5883: (52-72): Event with same name and parameter types defined twice.
