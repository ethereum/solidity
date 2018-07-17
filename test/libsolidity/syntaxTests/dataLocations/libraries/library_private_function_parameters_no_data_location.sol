library L {
    function f(uint[]) private pure {}
    function g(uint[]) internal pure {}
    function h(uint[]) public pure {}
}
// ----
// TypeError: (27-33): Location must be specified as either "memory" or "storage" for parameters.
