contract test {
    mapping(address owner => mapping(address owner => address hello)) world;
}
// ----
// DeclarationError 1809: (20-85): Conflicting parameter name "owner" in mapping.
