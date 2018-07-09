contract C {
    function h(uint[]) public pure {}
}
// ----
// TypeError: (28-34): Location must be specified as "memory" for parameters in publicly visible functions.
