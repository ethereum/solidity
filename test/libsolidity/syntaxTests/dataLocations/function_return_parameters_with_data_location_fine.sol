contract C {
    function f() private pure returns(uint[] memory, uint[] storage b) { b = b; }
    function g() internal pure returns(uint[] memory, uint[] storage b) { b = b; }
    function h() public pure returns(uint[] memory) {}
    function i() external pure returns(uint[] memory) {}
}
