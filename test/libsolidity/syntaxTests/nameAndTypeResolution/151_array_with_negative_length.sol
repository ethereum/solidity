contract c {
    function f(uint a) public { uint8[-1] x; }
}
// ----
// TypeError 3658: (51-53): Array with negative length specified.
// TypeError 6651: (45-56): Data location must be "storage", "memory" or "calldata" for variable, but none was given.
