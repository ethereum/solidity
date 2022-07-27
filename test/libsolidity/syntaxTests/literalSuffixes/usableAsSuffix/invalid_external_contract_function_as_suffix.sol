contract C {
    uint a = 1000 this.suffix;

    function suffix(uint x) external pure returns (uint) { return x; }
}
// ----
// TypeError 4438: (26-42): The literal suffix needs to be a pre-defined suffix or a file-level pure function.
