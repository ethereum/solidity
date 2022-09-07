pragma abicoder v2;

contract C {
    mapping (uint => uint8[][]) m;

    uint8[][] s;

    function from_calldata(uint8[][] calldata _a) public returns (uint8[][] memory) {
        m[0] = _a;
        return m[0];
    }
}

// ====
// compileViaYul: true
// ----
// from_calldata(uint8[][]): 0x20, 2, 0x40, 0xa0, 2, 10, 11, 3, 12, 13, 14 -> 0x20, 2, 0x40, 0xa0, 2, 10, 11, 3, 12, 13, 14
// gas irOptimized: 139927
