contract test {
    mapping(address hello => mapping(address owner => address owner)) world;
}
// ----
// DeclarationError 1809: (45-84): Conflicting parameter name "owner" in mapping.
