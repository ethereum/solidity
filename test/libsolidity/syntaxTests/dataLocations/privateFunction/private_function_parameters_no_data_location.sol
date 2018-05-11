contract C {
    function f(uint[]) private pure {}
    function g(uint[]) internal pure {}
    function h(uint[]) public pure {}
}
// ----
// TypeError: (28-34): Location must be specified as either "memory" or "storage" for parameters.
