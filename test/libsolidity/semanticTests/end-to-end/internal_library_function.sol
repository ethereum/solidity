// tests that internal library functions can be called from outside
// and retain the same memory context (i.e. are pulled into the caller's code)
// This has to work without linking, because everything will be inlined.
library L {
    function f(uint256[] memory _data) internal {
        _data[3] = 2;
    }
}


contract C {
    function f() public returns (uint256) {
        uint256[] memory x = new uint256[](7);
        x[3] = 8;
        L.f(x);
        return x[3];
    }
}

// ----
// f() -> 2
