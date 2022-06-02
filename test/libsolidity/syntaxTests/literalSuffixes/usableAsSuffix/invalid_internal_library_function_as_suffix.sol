library L {
    function suffix(uint x) internal pure returns (uint) { return x; }
}

contract C {
    uint x = 1000 L.suffix;
}
// ----
// TypeError 4438: (112-125): The literal suffix needs to be a pre-defined suffix or a file-level function.
