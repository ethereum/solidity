contract test {
    function f() public {
        uint[3.5] a; a;
    }
}
// ----
// TypeError 3208: (55-58='3.5'): Array with fractional length specified.
// TypeError 6651: (50-61='uint[3.5] a'): Data location must be "storage", "memory" or "calldata" for variable, but none was given.
