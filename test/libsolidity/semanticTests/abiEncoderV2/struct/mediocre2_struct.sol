pragma abicoder               v2;

contract C {
    struct S { C c; uint[] x; }
    function f(uint a, S[2] memory s1, uint b) public returns (uint r1, C r2, uint r3) {
        r1 = a;
        r2 = s1[0].c;
        r3 = b;
    }
}
// ----
// f(uint256,(address,uint256[])[2],uint256): 7, 0x60, 8, 0x40, 0xE0, 0x0, 0x40, 2, 0x11, 0x12, 0x99, 0x40, 4, 0x31, 0x32, 0x34, 0x35 -> 7, 0x0, 8
