// This has to work without linking, because everything will be inlined.
library L {
    struct S {
        uint256[] data;
    }

    function f(S memory _s) internal returns (uint256[] memory) {
        _s.data[3] = 2;
        return _s.data;
    }
}


contract C {
    using L for L.S;

    function f() public returns (uint256) {
        L.S memory x;
        x.data = new uint256[](7);
        x.data[3] = 8;
        return x.f()[3];
    }
}
// ----
// f() -> 2
