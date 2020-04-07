contract c {
    function f(uint a) public { uint8[-1] x; }
}
// ----
// TypeError: (51-53): Array with negative length specified.
// TypeError: (45-56): Data location must be "storage" or "memory" for variable, but none was given.
