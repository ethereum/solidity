contract C {
	int[] x;
	function f() public {
		int[] storage a = x;
		int[] memory b;
		a = b;
		a = int[](b);
	}
}
// ----
// TypeError: (93-94): Type int256[] memory is not implicitly convertible to expected type int256[] storage pointer.
// TypeError: (102-110): Type int256[] memory is not implicitly convertible to expected type int256[] storage pointer.
