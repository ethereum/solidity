contract test {
    function f(uint x, uint y) public returns (uint a) {}
    function g() public {
        function (uint, uint) internal returns (uint) f1 = f;
    }
}
// ----
// Warning: (108-156): Unused local variable.
// Warning: (20-73): Function state mutability can be restricted to pure
// Warning: (78-167): Function state mutability can be restricted to pure
