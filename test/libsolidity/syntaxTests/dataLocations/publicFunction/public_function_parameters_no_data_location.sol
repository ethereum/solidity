contract C {
    function h(uint[]) public pure {}
}
// ----
// TypeError: (28-34): Data location must be "memory" for parameter in function, but none was given.
