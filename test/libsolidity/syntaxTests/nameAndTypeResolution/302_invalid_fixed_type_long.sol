contract test {
    function f() public {
        fixed8x888888888888888888888888888888888888888888888888888 b;
    }
}
// ----
// DeclarationError 7920: (50-108): Identifier not found or not unique.
