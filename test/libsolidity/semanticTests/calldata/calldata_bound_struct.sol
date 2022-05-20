pragma abicoder v2;

struct S {
    uint x;
    uint y;
}

library L {
    function reverse(S calldata _s) internal pure returns (uint, uint) {
        return (_s.y, _s.x);
    }
}

contract C {
    using L for S;

    function test(uint, S calldata _s, uint) external pure returns (uint, uint) {
        return _s.reverse();
    }
}
// ----
// test(uint256,(uint256,uint256),uint256): 7, 66, 77, 4 -> 77, 66
