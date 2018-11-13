contract C {
    function g(uint[]) internal pure {}
}
// ----
// TypeError: (28-34): Data location must be "storage" or "memory" for parameter in function, but none was given.
