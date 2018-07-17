contract C {
    function g(uint[]) internal pure {}
}
// ----
// TypeError: (28-34): Location must be specified as either "memory" or "storage" for parameters.