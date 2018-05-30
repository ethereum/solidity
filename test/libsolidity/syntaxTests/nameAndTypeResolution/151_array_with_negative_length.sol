contract c {
    function f(uint a) public { uint8[-1] x; }
}
// ----
// TypeError: (51-53): Array with negative length specified.
