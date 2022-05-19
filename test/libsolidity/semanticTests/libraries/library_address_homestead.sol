library L {
    function f(uint256 a, uint256 b) external {
        assert(a * a == b);
    }
}
contract C {
    function addr() public view returns (bool) {
        return address(L) == address(0);
    }
    function g(uint256 a, uint256 b) public returns (bool) {
        (bool success,) = address(L).delegatecall(abi.encodeWithSignature("f(uint256,uint256)", a, b));
        return success;
    }
}
// ----
// library: L
// g(uint256,uint256): 1, 1 -> true
// g(uint256,uint256): 1, 2 -> false
// g(uint256,uint256): 2, 3 -> false
// g(uint256,uint256): 2, 4 -> true
// g(uint256,uint256): 2, 5 -> false
// g(uint256,uint256): 4, 15 -> false
// g(uint256,uint256): 4, 16 -> true
// g(uint256,uint256): 4, 17 -> false
