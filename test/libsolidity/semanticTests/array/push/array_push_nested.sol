contract C {
    uint8 b = 23;
    uint120[][] s;
    uint8 a = 17;
    function f() public {
        s.push();
        assert(s.length == 1);
        assert(s[0].length == 0);
        s[0].push();
        assert(s[0].length == 1);
        assert(s[0][0] == 0);
    }
}
// ----
// f() ->
