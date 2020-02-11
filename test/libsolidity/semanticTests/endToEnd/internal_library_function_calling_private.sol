library L {
    function g(uint[] memory _data) private {
        _data[3] = 2;
    }

    function f(uint[] memory _data) internal {
        g(_data);
    }
}
contract C {
    function f() public returns(uint) {
        uint[] memory x = new uint[](7);
        x[3] = 8;
        L.f(x);
        return x[3];
    }
}

// ----
// f() -> 2
