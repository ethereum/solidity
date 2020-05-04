contract test {
    function f() public {
        uint[ufixed(3.5)] a; a;
    }
}
// ----
// TypeError: (55-66): Invalid array length, expected integer literal or constant expression.
// TypeError: (50-69): Data location must be "storage" or "memory" for variable, but none was given.
