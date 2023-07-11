contract C {
    modifier mod1 {
        uint8 a = 1;
        uint8 b = 2;
        _;
    }
    modifier mod2(bool a) {
        if (a) return;
        else _;
    }

    function f(bool a) public mod1 mod2(a) returns (uint256 r) {
        return 3;
    }
}
// ----
// f(bool): true -> 0
// f(bool): false -> 3
