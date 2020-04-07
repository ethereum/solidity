contract test {
    function f() public {
        uint[fixed(3.5)] a; a;
    }
}
// ----
// TypeError: (55-65): Invalid array length, expected integer literal or constant expression.
// TypeError: (50-68): Data location must be "storage" or "memory" for variable, but none was given.
