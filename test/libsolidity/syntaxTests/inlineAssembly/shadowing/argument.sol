contract C {
    function f(uint a) public pure {
        assembly {
            let a := 1
        }
    }
}
// ----
// DeclarationError: (85-86): This declaration shadows a declaration outside the inline assembly block.
