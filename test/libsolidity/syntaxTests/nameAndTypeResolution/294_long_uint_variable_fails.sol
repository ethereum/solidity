contract test {
    function f() public {
        uint99999999999999999999999999 something = 3;
    }
}
// ----
// DeclarationError 7920: (50-80='uint99999999999999999999999999'): Identifier not found or not unique.
