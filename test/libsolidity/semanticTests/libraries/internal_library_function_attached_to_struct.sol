// This has to work without linking, because everything will be inlined.
library L {
    struct S {
        uint256[] data;
    }

    function f(S memory _s) internal {
        _s.data[3] = 2;
    }
}


contract C {
    using L for L.S;

    function f() public returns (uint256) {
        L.S memory x;
        x.data = new uint256[](7);
        x.data[3] = 8;
        x.f();
        return x.data[3];
    }
}
// ----
// f() -> 2
