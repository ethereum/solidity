contract c {
    function f(uint a) public { uint8[a] x; }
}
// ----
// TypeError 5462: (51-52='a'): Invalid array length, expected integer literal or constant expression.
// TypeError 6651: (45-55='uint8[a] x'): Data location must be "storage", "memory" or "calldata" for variable, but none was given.
