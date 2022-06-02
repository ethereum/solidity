contract C {
    uint public g;

    uint a = 1 this.g;
}
// ----
// DeclarationError 7920: (48-54): Identifier not found or not unique.
