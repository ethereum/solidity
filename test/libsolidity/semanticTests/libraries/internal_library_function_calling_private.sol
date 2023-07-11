// tests that internal library functions that are called from outside and that
// themselves call private functions are still able to (i.e. the private function
// also has to be pulled into the caller's code)
// This has to work without linking, because everything will be inlined.
library L {
    function g(uint256[] memory _data) private {
        _data[3] = 2;
    }

    function f(uint256[] memory _data) internal {
        g(_data);
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
