contract C {
    uint constant a;
    function f() public pure {
        assembly {
            let a := 1
        }
    }
}
// ----
// DeclarationError 3859: (100-101): This declaration shadows a declaration outside the inline assembly block.
