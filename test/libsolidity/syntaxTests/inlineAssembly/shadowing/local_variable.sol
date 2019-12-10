contract C {
    function f() public pure {
        uint a;
        assembly {
            let a := 1
        }
    }
}
// ----
// DeclarationError: (95-96): This declaration shadows a declaration outside the inline assembly block.
