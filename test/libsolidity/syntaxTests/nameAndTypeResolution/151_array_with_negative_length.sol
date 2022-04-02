contract c {
    function f(uint a) public { uint8[-1] x; }
}
// ----
// TypeError 3658: (51-53='-1'): Array with negative length specified.
// TypeError 6651: (45-56='uint8[-1] x'): Data location must be "storage", "memory" or "calldata" for variable, but none was given.
