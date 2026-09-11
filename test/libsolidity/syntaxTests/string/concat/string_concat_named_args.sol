contract C {
    function f() public pure {
        string.concat({x: "abc", y: "def"});
    }
}
// ----
// TypeError 4903: (52-87): Named arguments cannot be used with string.concat().
