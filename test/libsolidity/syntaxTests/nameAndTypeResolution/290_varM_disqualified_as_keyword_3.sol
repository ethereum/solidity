contract test {
    function f() public {
        bytesM fail = "now";
    }
}
// ----
// DeclarationError 7920: (50-56='bytesM'): Identifier not found or not unique.
