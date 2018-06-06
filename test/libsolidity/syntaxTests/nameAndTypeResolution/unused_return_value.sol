contract test {
    function g() public returns (uint) {}
    function f() public {
        g();
    }
}
// ----
// Warning: (20-57): Function state mutability can be restricted to pure
