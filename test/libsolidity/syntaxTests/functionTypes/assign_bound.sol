library L {
    function foo(uint256 a, uint256 b) internal pure returns (uint256) {
        return a + b;
    }
}
contract C {
    using L for uint256;

    function bar() public {
        uint256 x;
        function (uint256, uint256) internal pure returns (uint256) ptr = x.foo;
    }
}
// ----
// TypeError 9574: (209-280): Type function (uint256,uint256) pure returns (uint256) is not implicitly convertible to expected type function (uint256,uint256) pure returns (uint256). Bound functions can not be converted to non-bound functions.
