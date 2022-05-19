library L {
    // NOTE: External function takes up two stack slots
    function double(function(uint) external pure returns (uint) f, uint x) internal pure returns (uint) {
        return f(x) * 2;
    }
}

contract C {
    using L for function(uint) external pure returns (uint);

    function identity(uint x) external pure returns (uint) {
        return x;
    }

    function test(uint value) public returns (uint) {
        return this.identity.double(value);
    }
}

// ----
// test(uint256): 5 -> 10
