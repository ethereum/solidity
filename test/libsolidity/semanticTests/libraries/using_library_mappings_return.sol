library Lib {
    function choose(mapping(uint => mapping(uint => uint)) storage m, uint key) external returns (mapping(uint => uint) storage) {
        return m[key];
    }
}
contract Test {
    mapping(uint => mapping(uint => uint)) m;
    function f() public returns (uint, uint, uint, uint, uint, uint)
    {
        Lib.choose(m, 0)[0] = 1;
        Lib.choose(m, 0)[2] = 42;
        Lib.choose(m, 1)[0] = 23;
        Lib.choose(m, 1)[2] = 99;
        return (m[0][0], m[0][1], m[0][2], m[1][0], m[1][1], m[1][2]);
    }
}
// ====
// compileToEwasm: false
// ----
// library: Lib
// f() -> 1, 0, 0x2a, 0x17, 0, 0x63
// gas irOptimized: 120265
// gas legacy: 125109
// gas legacyOptimized: 120128
