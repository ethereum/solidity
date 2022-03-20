contract C {
    function f() public pure {
        assembly {
            function f() {}
        }
    }
}
// ----
// DeclarationError 3859: (75-90): This declaration shadows a declaration outside the inline assembly block.
