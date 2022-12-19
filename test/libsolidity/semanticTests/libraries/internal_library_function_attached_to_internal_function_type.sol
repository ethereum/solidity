library L {
    function double(function(uint) internal pure returns (uint) f, uint x) internal pure returns (uint) {
        return f(x) * 2;
    }
}

contract C {
    using L for function(uint) internal pure returns (uint);

    function identity(uint x) internal pure returns (uint) {
        return x;
    }

    function test(uint value) public returns (uint) {
        return identity.double(value);
    }
}

// ----
// test(uint256): 5 -> 10
