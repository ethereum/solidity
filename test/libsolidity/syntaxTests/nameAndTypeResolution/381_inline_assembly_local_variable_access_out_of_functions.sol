contract test {
    function f() public {
        uint a;
        assembly {
            function g() -> x { x := a }
        }
    }
}
// ----
// DeclarationError: (114-115): Cannot access local Solidity variables from inside an inline assembly function.
