contract test {
    event A(uint i);
    event A(uint indexed i);
}
// ----
// DeclarationError: (20-36): Event with same name and arguments defined twice.
// TypeError: (20-36): Event overload must extend parameter list or have different types.
