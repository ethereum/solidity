contract test {
    mapping(address owner => mapping(address owner => address owner)) owner;
}
// ----
// DeclarationError 1809: (45-84): Conflicting parameter name "owner" in mapping.
// DeclarationError 1809: (20-85): Conflicting parameter name "owner" in mapping.
// DeclarationError 1809: (20-85): Conflicting parameter name "owner" in mapping.
