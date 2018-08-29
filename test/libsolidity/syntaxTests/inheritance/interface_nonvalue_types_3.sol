pragma experimental ABIEncoderV2;

interface FooI {
	struct S { uint x; }
    function fooNums(S calldata nums) external;
}

contract Foo is FooI {
    function fooNums(S memory nums) public {}
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// Warning: (169-182): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning: (152-193): Function state mutability can be restricted to pure
