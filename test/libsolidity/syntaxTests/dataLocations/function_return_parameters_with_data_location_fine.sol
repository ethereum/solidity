contract C {
    function f() private pure returns(uint[] memory, uint[] storage b) { b = b; }
    function g() internal pure returns(uint[] memory, uint[] storage b) { b = b; }
    function h() public pure returns(uint[] memory) {}
    function i() external pure returns(uint[] memory) {}
}
// ----
// Warning 6321: (51-64): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// Warning 6321: (134-147): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
