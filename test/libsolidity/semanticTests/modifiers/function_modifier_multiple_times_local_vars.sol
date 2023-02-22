contract C {
    uint256 public a;
    modifier mod(uint256 x) {
        uint256 b = x;
        a += b;
        _;
        a -= b;
        assert(b == x);
    }

    function f(uint256 x) public mod(2) mod(5) mod(x) returns (uint256) {
        return a;
    }
}
// ----
// f(uint256): 3 -> 10
// a() -> 0
