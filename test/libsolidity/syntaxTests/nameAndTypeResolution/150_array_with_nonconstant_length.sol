contract c {
    function f(uint a) public { uint8[a] x; }
}
// ----
// TypeError: (51-52): Invalid array length, expected integer literal or constant expression.
// TypeError: (45-55): Data location must be "storage" or "memory" for variable, but none was given.
