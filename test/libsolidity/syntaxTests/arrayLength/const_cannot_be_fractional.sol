contract C {
    fixed constant L = 10.5;
    uint[L] ids;
}
// ----
// TypeError: (51-52): Array with fractional length specified.
