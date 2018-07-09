contract C {
    // Shows that the warning for no data location provided can be silenced with storage or memory.
    function f() private pure returns(uint[] memory, uint[] storage b) { b = b; }
    function g() internal pure returns(uint[] memory, uint[] storage b) { b = b; }
    function h() public pure returns(uint[] memory) {}
    function i() external pure returns(uint[] memory) {}
}