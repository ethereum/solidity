contract C {
    function g(uint[]) internal pure {}
}
// ----
// TypeError: (28-34): Storage location must be "storage" or "memory" for parameter in internal function, but none was given.
