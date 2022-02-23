contract test {
    function f() public {
        uint[ufixed(3.5)] a; a;
    }
}
// ----
// TypeError 5462: (55-66): Invalid array length, expected integer literal or constant expression.
// TypeError 6651: (50-69): Data location must be "storage", "memory" or "calldata" for variable, but none was given.
