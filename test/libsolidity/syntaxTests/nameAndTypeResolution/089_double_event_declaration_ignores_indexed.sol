contract test {
    event A(uint i);
    event A(uint indexed i);
}
// ----
// DeclarationError 5883: (20-36): Event with same name and parameter types defined twice.
