contract test {
    function g() public { f(); }
    function f() public {}
}
// ----
// Warning: (53-75): Function state mutability can be restricted to pure
