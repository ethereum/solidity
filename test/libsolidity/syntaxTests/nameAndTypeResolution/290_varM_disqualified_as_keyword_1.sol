contract test {
    function f() public {
        uintM something = 3;
    }
}
// ----
// DeclarationError: (50-55): Identifier not found or not unique.
