library L {
    function suffix(uint x) external pure returns (uint) { return x; }
}

contract C {
    uint x = 1000 L.suffix;
}
// ----
// TypeError 4438: (112-125): The literal suffix must be either a subdenomination or a file-level suffix function.
