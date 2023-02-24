contract C {
    uint a = 1000 suffix;

    modifier suffix(uint x) { _; }
}
// ----
// TypeError 4438: (26-37): The literal suffix must be either a subdenomination or a file-level suffix function.
