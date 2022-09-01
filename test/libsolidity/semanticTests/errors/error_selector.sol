library L {
    error E();
}
library S {
    error E(uint);
}
library T {
    error E();
}

error E();

interface I {
    error E();
    function f() external pure;
}

contract D {
    error F();
}

contract C is D {
    function test1() public pure returns (bytes4, bytes4, bytes4, bytes4) {
        assert(L.E.selector == T.E.selector);
        assert(L.E.selector != S.E.selector);
        assert(E.selector == L.E.selector);
        assert(I.E.selector == L.E.selector);
        return (L.E.selector, S.E.selector, E.selector, I.E.selector);
    }

    bytes4 s1 = L.E.selector;
    bytes4 s2 = S.E.selector;
    bytes4 s3 = T.E.selector;
    bytes4 s4 = I.E.selector;
    function test2() external returns (bytes4, bytes4, bytes4, bytes4) {
        return (s1, s2, s3, s4);
    }

    function test3() external returns (bytes4) {
        return (F.selector);
    }
}
// ====
// compileViaYul: also
// ----
// test1() -> 0x92bbf6e800000000000000000000000000000000000000000000000000000000, 0x2ff06700000000000000000000000000000000000000000000000000000000, 0x92bbf6e800000000000000000000000000000000000000000000000000000000, 0x92bbf6e800000000000000000000000000000000000000000000000000000000
// test2() -> 0x92bbf6e800000000000000000000000000000000000000000000000000000000, 0x2ff06700000000000000000000000000000000000000000000000000000000, 0x92bbf6e800000000000000000000000000000000000000000000000000000000, 0x92bbf6e800000000000000000000000000000000000000000000000000000000
// test3() -> 0x28811f5900000000000000000000000000000000000000000000000000000000
