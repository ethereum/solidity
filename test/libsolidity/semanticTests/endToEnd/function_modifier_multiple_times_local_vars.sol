contract C {
    uint public a;
    modifier mod(uint x) {
        uint b = x;
        a += b;
        _;
        a -= b;
        assert(b == x);
    }

    function f(uint x) mod(2) mod(5) mod(x) public returns(uint) {
        return a;
    }
}

// ----
// f(uint256): 3 -> 10
// a() -> 0
