contract C {
    uint y;
    function g(bool x) internal returns (bool) {
        y = 42;
        return x;
    }
    error E(uint256);
    function h() internal returns (uint256) { return y; }
    function f(bool c) public {
        require(g(c), E(h()));
    }
}

// ----
// f(bool): false -> FAILURE, hex"002ff067", 42
// f(bool): true ->
