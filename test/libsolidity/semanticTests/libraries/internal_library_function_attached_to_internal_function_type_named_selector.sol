library L {
    function selector(function(uint) internal pure returns (uint) f, uint x) internal pure returns (uint) {
        return f(x) * 2;
    }
}

contract C {
    using L for function(uint) internal pure returns (uint);

    function identity(uint x) internal pure returns (uint) {
        return x;
    }

    function test(uint value) public returns (uint) {
        return identity.selector(value);
    }
}
// ----
// test(uint256): 5 -> 10
