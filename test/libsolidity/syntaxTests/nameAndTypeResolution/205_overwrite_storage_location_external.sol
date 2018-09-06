contract C {
    function f(uint[] storage a) external {}
}
// ----
// TypeError: (28-44): Data location must be "calldata" for parameter in external function, but "storage" was given.
