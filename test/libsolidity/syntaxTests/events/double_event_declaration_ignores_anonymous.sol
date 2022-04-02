contract test {
    event A(uint i);
    event A(uint i) anonymous;
}
// ----
// DeclarationError 5883: (20-36='event A(uint i);'): Event with same name and parameter types defined twice.
