contract C {
    function f(uint x) public returns(uint[256] memory r) {
        r[uint8(x)] = 2;
    }
}

// ----
f(uint256): "1"
f(uint256): "257"
