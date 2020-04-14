contract test {
    function f() public {
        uint[3.5] a; a;
    }
}
// ----
// TypeError: (55-58): Array with fractional length specified.
// TypeError: (50-61): Data location must be "storage" or "memory" for variable, but none was given.
