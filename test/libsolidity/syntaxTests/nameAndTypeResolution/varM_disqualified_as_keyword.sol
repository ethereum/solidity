contract test {
    function f() public {
        uintM something = 3;
        intM should = 4;
        bytesM fail = "now";
    }
}
// ----
// DeclarationError: (50-55): Identifier not found or not unique.
// DeclarationError: (79-83): Identifier not found or not unique.
// DeclarationError: (104-110): Identifier not found or not unique.
