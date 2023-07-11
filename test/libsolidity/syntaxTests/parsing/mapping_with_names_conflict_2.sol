contract test {
    mapping(address owner => address owner) owner;
}
// ----
// DeclarationError 1809: (20-59): Conflicting parameter name "owner" in mapping.
