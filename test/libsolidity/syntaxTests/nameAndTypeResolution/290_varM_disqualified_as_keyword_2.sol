contract test {
    function f() public {
        intM should = 4;
    }
}
// ----
// DeclarationError: (50-54): Identifier not found or not unique.
