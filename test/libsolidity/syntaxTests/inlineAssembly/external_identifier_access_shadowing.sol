contract C {
    function f() public returns (uint x) {
        assembly {
            function g() -> x {
                x := 42
            }
            x := g()
        }
    }
}
// ----
// DeclarationError 6578: (123-124): Cannot access local Solidity variables from inside an inline assembly function.
