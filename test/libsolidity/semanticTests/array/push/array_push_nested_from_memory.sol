contract C {
    uint8 b = 23;
    uint120[][] s;
    uint8 a = 17;
    function f() public returns(uint120) {
        delete s;
        uint120[] memory m = new uint120[](3);
        m[0] = 1;
        s.push(m);
        assert(s.length == 1);
        assert(s[0].length == m.length);
        assert(s[0].length > 0);
        return s[0][0];
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 1
