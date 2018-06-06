contract C {
    uint x;
    function(uint a) public { x = 2; }
}
// ----
// TypeError: (37-45): Fallback function cannot take parameters.
