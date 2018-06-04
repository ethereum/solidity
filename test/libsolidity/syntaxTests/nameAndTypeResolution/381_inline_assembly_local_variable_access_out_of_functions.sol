pragma experimental "v0.5.0";
contract test {
    function f() public {
        uint a;
        assembly {
            function g() -> x { x := a }
        }
    }
}
// ----
// DeclarationError: (144-145): Cannot access local Solidity variables from inside an inline assembly function.
