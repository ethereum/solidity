pragma abicoder v2;

contract C {
    mapping (uint => uint8[][]) m;

    uint8[][] s;

    constructor() {
        s = new uint8[][](2);

        s[0] = new uint8[](2);
        s[0][0] = 10;
        s[0][1] = 11;

        s[1] = new uint8[](3);
        s[1][0] = 12;
        s[1][1] = 13;
        s[1][2] = 14;
    }

    function from_storage() public returns (uint8[][] memory) {
        m[0] = new uint8[][](2);
        m[0][0] = s[0];
        m[0][1] = s[1];
        return m[0];
    }


    function from_storage_ptr() public returns (uint8[][] memory) {
        uint8[][] storage sPtr = s;
        m[0] = new uint8[][](2);
        m[0][0] = sPtr[0];
        m[0][1] = sPtr[1];
        return m[0];
    }


    function from_memory() public returns (uint8[][] memory) {
        uint8[][] memory a = s;
        m[0] = new uint8[][](2);
        m[0][0] = a[0];
        m[0][1] = a[1];
        return m[0];
    }

    function from_calldata(uint8[][] calldata _a) public returns (uint8[][] memory) {
        m[0] = new uint8[][](2);
        m[0][0] = _a[0];
        m[0][1] = _a[1];
        return m[0];
    }
}

// ----
// from_storage() -> 0x20, 2, 0x40, 0xa0, 2, 10, 11, 3, 12, 13, 14
// gas irOptimized: 150098
// gas legacy: 150830
// gas legacyOptimized: 148728
// from_storage_ptr() -> 0x20, 2, 0x40, 0xa0, 2, 10, 11, 3, 12, 13, 14
// from_memory() -> 0x20, 2, 0x40, 0xa0, 2, 10, 11, 3, 12, 13, 14
// from_calldata(uint8[][]): 0x20, 2, 0x40, 0xa0, 2, 10, 11, 3, 12, 13, 14 -> 0x20, 2, 0x40, 0xa0, 2, 10, 11, 3, 12, 13, 14
