contract test {
    function f() public {
        uint a;
        assembly {
            function g() -> x { x := a }
        }
    }
}
// ----
// DeclarationError 6578: (114-115='a'): Cannot access local Solidity variables from inside an inline assembly function.
