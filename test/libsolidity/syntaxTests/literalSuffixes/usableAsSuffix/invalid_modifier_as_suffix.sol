contract C {
    uint a = 1000 suffix;

    modifier suffix(uint x) { _; }
}
// ----
// TypeError 4438: (26-37): The literal suffix needs to be a pre-defined suffix or a file-level pure function.
