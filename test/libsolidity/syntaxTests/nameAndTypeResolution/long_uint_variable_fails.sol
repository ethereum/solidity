contract test {
    function f() public {
        uint99999999999999999999999999 something = 3;
    }
}
// ----
// DeclarationError: (50-80): Identifier not found or not unique.
