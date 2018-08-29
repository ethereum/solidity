interface FooI {
    function fooNum(uint num) external;
    function fooNums(uint[] calldata nums) external;
    function fooNums2(uint[] calldata nums) external;
}

contract Foo is FooI {
    function fooNum(uint num) public {}
    function fooNums(uint[] memory nums) public {}
}
// ----
// Warning: (210-218): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (251-269): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (234-280): Function state mutability can be restricted to pure
