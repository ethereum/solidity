contract base {
    event e(uint a, bytes3 indexed s, bool indexed b);
}
contract c is base {
    function f() public { emit e(2, "abc", true); }
}
// ----
