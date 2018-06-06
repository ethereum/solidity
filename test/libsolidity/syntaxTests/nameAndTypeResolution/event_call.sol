contract c {
    event e(uint a, bytes3 indexed s, bool indexed b);
    function f() public { e(2, "abc", true); }
}
// ----
// Warning: (94-111): Invoking events without "emit" prefix is deprecated.
