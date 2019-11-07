contract C {
    function f() public pure {
        assembly {
            let C := 1
        }
    }
}
// ----
// DeclarationError: (79-80): This declaration shadows a declaration outside the inline assembly block.
