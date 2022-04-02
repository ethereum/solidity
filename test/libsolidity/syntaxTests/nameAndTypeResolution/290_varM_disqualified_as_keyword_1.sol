contract test {
    function f() public {
        uintM something = 3;
    }
}
// ----
// DeclarationError 7920: (50-55='uintM'): Identifier not found or not unique.
