pragma abicoder v2;

contract C {
    struct S {
        bytes b;
        uint16[] a;
        uint16 u;
    }

    constructor() {
        uint16[] memory a = new uint16[](2);
        a[0] = 13;
        a[1] = 14;

        m[7] = S({b: "foo", a: a, u: 7});
    }

    mapping (uint => S) m;
    S s;

    function to_state() public returns (S memory) {
	s = m[7];
        return s;
    }

    function to_storage() public returns (S memory) {
        S storage sLocal = s;
	sLocal = m[7];
        return sLocal;
    }

    function to_memory() public returns (S memory) {
	return m[7];
    }

}

// ----
// to_state() -> 0x20, 0x60, 0xa0, 7, 3, 0x666F6F0000000000000000000000000000000000000000000000000000000000, 2, 13, 14
// gas irOptimized: 121513
// gas legacy: 123120
// gas legacyOptimized: 121776
// to_storage() -> 0x20, 0x60, 0xa0, 7, 3, 0x666F6F0000000000000000000000000000000000000000000000000000000000, 2, 13, 14
// to_memory() -> 0x20, 0x60, 0xa0, 7, 3, 0x666F6F0000000000000000000000000000000000000000000000000000000000, 2, 13, 14
