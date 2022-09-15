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
	s.b = m[7].b;
	s.a = m[7].a;
	s.u = m[7].u;
        return s;
    }

    function to_storage() public returns (S memory) {
        S storage sLocal = s;
	sLocal.b = m[7].b;
	sLocal.a = m[7].a;
	sLocal.u = m[7].u;
        return sLocal;
    }

    function to_memory() public returns (S memory) {
        S memory sLocal;
        sLocal.b = m[7].b;
        sLocal.a = m[7].a;
        sLocal.u = m[7].u;
	return sLocal;
    }

}

// ----
// to_state() -> 0x20, 0x60, 0xa0, 7, 3, 0x666F6F0000000000000000000000000000000000000000000000000000000000, 2, 13, 14
// gas irOptimized: 121616
// gas legacy: 123263
// gas legacyOptimized: 121785
// to_storage() -> 0x20, 0x60, 0xa0, 7, 3, 0x666F6F0000000000000000000000000000000000000000000000000000000000, 2, 13, 14
// to_memory() -> 0x20, 0x60, 0xa0, 7, 3, 0x666F6F0000000000000000000000000000000000000000000000000000000000, 2, 13, 14
