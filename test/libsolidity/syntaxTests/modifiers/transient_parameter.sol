contract A {
    modifier mod2(uint[] transient) { _; }
}
// ----
// TypeError 6651: (31-47): Data location must be "storage", "memory" or "calldata" for parameter in function, but none was given.
