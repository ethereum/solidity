contract test {
    event A(uint i);
    event A(uint i) anonymous;
}
// ----
// DeclarationError: (20-36): Event with same name and arguments defined twice.
