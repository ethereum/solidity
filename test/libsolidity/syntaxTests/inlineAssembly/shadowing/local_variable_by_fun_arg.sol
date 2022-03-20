contract C {
    function f() public pure {
        uint a;
        assembly {
            function g(a) {}
        }
    }
}
// ----
// DeclarationError 3859: (102-103): This declaration shadows a declaration outside the inline assembly block.
