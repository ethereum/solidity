contract test {
    function f() public {
        uint[fixed(3.5)] a; a;
    }
}
// ----
// TypeError 5462: (55-65='fixed(3.5)'): Invalid array length, expected integer literal or constant expression.
// TypeError 6651: (50-68='uint[fixed(3.5)] a'): Data location must be "storage", "memory" or "calldata" for variable, but none was given.
