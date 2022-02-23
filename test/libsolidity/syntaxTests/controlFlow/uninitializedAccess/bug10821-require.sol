contract Test {
	function testFunc() external {
		require(true);
		bytes storage b;
		b[0] = 0x42;
	}
}
// ----
// TypeError 3464: (86-87): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
