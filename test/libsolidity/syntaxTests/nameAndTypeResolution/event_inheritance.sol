contract base {
    event e(uint a, bytes3 indexed s, bool indexed b);
}
contract c is base {
    function f() public { e(2, "abc", true); }
}
// ----
// Warning: (120-137): Invoking events without "emit" prefix is deprecated.
