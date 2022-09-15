pragma abicoder v2;

contract C {
    struct S {
        bytes b;
        uint16[] a;
        uint16 u;
    }

    S s;
    constructor() {
        uint16[] memory a = new uint16[](2);
        a[0] = 13;
        a[1] = 14;

        s.b = "foo";
        s.a = a;
        s.u = 21;
    }

    mapping (uint => S) m;

    function from_state() public returns (S memory) {
        m[0] = s;
        return m[0];
    }

    function from_storage() public returns (S memory) {
        S storage sLocal = s;
        m[1] = sLocal;
        return m[1];
    }

    function from_memory() public returns (S memory) {
        S memory sMemory = s;
        m[2] = sMemory;
        return m[2];
    }


    function from_calldata(S calldata sCalldata) public returns (S memory) {
        m[3] = sCalldata;
        return m[3];
    }
}

// ----
// from_state() -> 0x20, 0x60, 0xa0, 21, 3, 0x666F6F0000000000000000000000000000000000000000000000000000000000, 2, 13, 14
// gas irOptimized: 121709
// gas legacy: 123198
// gas legacyOptimized: 121830
// from_storage() -> 0x20, 0x60, 0xa0, 21, 3, 0x666F6F0000000000000000000000000000000000000000000000000000000000, 2, 13, 14
// gas irOptimized: 121753
// gas legacy: 123247
// gas legacyOptimized: 121882
// from_memory() -> 0x20, 0x60, 0xa0, 21, 3, 0x666F6F0000000000000000000000000000000000000000000000000000000000, 2, 13, 14
// gas irOptimized: 122954
// gas legacy: 130146
// gas legacyOptimized: 128779
// from_calldata((bytes,uint16[],uint16)): 0x20, 0x60, 0xa0, 21, 3, 0x666F6F0000000000000000000000000000000000000000000000000000000000, 2, 13, 14 -> 0x20, 0x60, 0xa0, 21, 3, 0x666f6f0000000000000000000000000000000000000000000000000000000000, 2, 13, 14
// gas irOptimized: 115067
// gas legacy: 118383
// gas legacyOptimized: 115458
