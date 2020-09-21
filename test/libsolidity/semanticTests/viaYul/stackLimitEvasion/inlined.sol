contract C {
    uint256[1024] s;
    function f() public returns (uint256 x) {
        x = 42;
        uint256 x0 = s[0];
        uint256 x1 = s[1];
        uint256 x2 = s[2];
        uint256 x3 = s[3];
        uint256 x4 = s[4];
        uint256 x5 = s[5];
        uint256 x6 = s[6];
        uint256 x7 = s[7];
        uint256 x8 = s[8];
        uint256 x9 = s[9];
        uint256 x10 = s[10];
        uint256 x11 = s[11];
        uint256 x12 = s[12];
        uint256 x13 = s[13];
        uint256 x14 = s[14];
        uint256 x15 = s[15];
        uint256 x16 = s[16];
        uint256 x17 = s[17];
        uint256 x18 = s[18];
        s[1000] = x0 + 2;
        s[118] = x18;
        s[117] = x17;
        s[116] = x16;
        s[115] = x15;
        s[114] = x14;
        s[113] = x13;
        s[112] = x12;
        s[111] = x11;
        s[110] = x10;
        s[109] = x9;
        s[108] = x8;
        s[107] = x7;
        s[106] = x6;
        s[105] = x5;
        s[104] = x4;
        s[103] = x3;
        s[102] = x2;
        s[101] = x1;
        s[100] = x0;
    }
    function test() public view returns(uint256) {
        return s[1000];
    }
}
// ====
// compileViaYul: true
// ----
// f() -> 0x2a
// test() -> 2
