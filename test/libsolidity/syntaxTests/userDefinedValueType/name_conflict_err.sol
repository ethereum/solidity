type MyInt is int;
type MyInt is address;
contract C {
    type MyAddress is address;
    type MyAddress is address;
}
// ----
// DeclarationError 2333: (19-41='type MyInt is address;'): Identifier already declared.
// DeclarationError 2333: (90-116='type MyAddress is address;'): Identifier already declared.
