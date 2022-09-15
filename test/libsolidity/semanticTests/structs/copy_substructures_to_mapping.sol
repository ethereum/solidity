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

    function from_memory() public returns (S memory) {
        S memory sMemory = s;
        m[0].b = sMemory.b;
        m[0].a = sMemory.a;
        m[0].u = sMemory.u;
        return m[0];
    }

    function from_state() public returns (S memory) {
        m[1].b = s.b;
        m[1].a = s.a;
        m[1].u = s.u;
        return m[1];
    }

    function from_storage() public returns (S memory) {
        S storage sLocal = s;
        m[1].b = sLocal.b;
        m[1].a = sLocal.a;
        m[1].u = sLocal.u;
        return m[1];
    }

    function from_calldata(S calldata sCalldata) public returns (S memory) {
        m[2].b = sCalldata.b;
        m[2].a = sCalldata.a;
        m[2].u = sCalldata.u;
        return m[2];
    }
}

// ----
// from_memory() -> 0x20, 0x60, 0xa0, 0x15, 3, 0x666F6F0000000000000000000000000000000000000000000000000000000000, 2, 13, 14
// gas irOptimized: 123062
// gas legacy: 130289
// gas legacyOptimized: 128785
// from_state() -> 0x20, 0x60, 0xa0, 21, 3, 0x666F6F0000000000000000000000000000000000000000000000000000000000, 2, 13, 14
// gas irOptimized: 121776
// gas legacy: 123341
// gas legacyOptimized: 121892
// from_calldata((bytes,uint16[],uint16)): 0x20, 0x60, 0xa0, 21, 3, 0x666F6F0000000000000000000000000000000000000000000000000000000000, 2, 13, 14 -> 0x20, 0x60, 0xa0, 0x15, 3, 0x666F6F0000000000000000000000000000000000000000000000000000000000, 2, 13, 14
// gas irOptimized: 115169
// gas legacy: 122579
// gas legacyOptimized: 120829
