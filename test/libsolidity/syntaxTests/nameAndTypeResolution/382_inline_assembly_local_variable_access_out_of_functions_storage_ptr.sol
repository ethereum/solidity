pragma experimental "v0.5.0";
contract test {
    uint[] r;
    function f() public {
        uint[] storage a = r;
        assembly {
            function g() -> x { x := a_offset }
        }
    }
}
// ----
// DeclarationError: (172-180): Cannot access local Solidity variables from inside an inline assembly function.
