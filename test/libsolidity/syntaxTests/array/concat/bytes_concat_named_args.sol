contract C {
    function f() public pure {
        bytes.concat({x: "abc", y: hex"00"});
    }
}
// ----
// TypeError 8145: (52-88): Named arguments cannot be used with bytes.concat().
