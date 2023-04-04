contract C {
    uint a = 1000 suffix;

    modifier suffix(uint x) { _; }
}
// ----
// TypeError 5704: (26-37): Modifier cannot be used as a literal suffix.
