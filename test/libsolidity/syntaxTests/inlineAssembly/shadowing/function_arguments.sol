contract C {
    uint x;
    function f() public pure {
        assembly {
            function g(f) -> x {}
        }
    }
}
// ----
// DeclarationError 3859: (98-99): This declaration shadows a declaration outside the inline assembly block.
// DeclarationError 3859: (104-105): This declaration shadows a declaration outside the inline assembly block.
