contract c {
    event e(uint indexed a, bytes3 indexed s, bool indexed b);
    function f() public { e(2, "abc", true); }
}
// ----
// Warning: (102-119): Invoking events without "emit" prefix is deprecated.
