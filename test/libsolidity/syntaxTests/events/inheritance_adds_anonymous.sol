contract A {
    event X();
}
contract B is A {
    event X() anonymous;
}
// ----
// DeclarationError 5883: (52-72='event X() anonymous;'): Event with same name and parameter types defined twice.
