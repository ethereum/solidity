contract test {
    function f() public {
        bytes32 bytes10abc = "abc";
    }
}
// ----
// Warning: (50-68): Unused local variable.
// Warning: (20-83): Function state mutability can be restricted to pure
