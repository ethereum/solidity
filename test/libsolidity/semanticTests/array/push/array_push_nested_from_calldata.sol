contract C {
    uint8 b = 23;
    uint120[][] s;
    uint8 a = 17;
    function f(uint120[] calldata c) public returns(uint120) {
        s.push(c);
        assert(s.length == 1);
        assert(s[0].length == c.length);
        assert(s[0].length > 0);
        return s[0][0];
    }
}
// ----
// f(uint120[]): 0x20, 3, 1, 2, 3 -> 1
// gas irOptimized: 112812
// gas legacy: 113659
// gas legacyOptimized: 113482
