contract test {
    uint[] r;
    function f() public {
        uint[] storage a = r;
        assembly {
            function g() -> x { x := a.offset }
        }
    }
}
// ----
// DeclarationError 6578: (142-150): Cannot access local Solidity variables from inside an inline assembly function.
