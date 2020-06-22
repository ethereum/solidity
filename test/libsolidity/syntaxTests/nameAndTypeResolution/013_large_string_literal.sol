contract test {
    function f() public { string memory x = "123456789012345678901234567890123"; }
}
// ----
// Warning 2072: (42-57): Unused local variable.
// Warning 2018: (20-98): Function state mutability can be restricted to pure
