contract C {
	function f() public {
		uint[] storage x;
		uint[10] storage y;
		x;
		y;
	}
}
// ----
// TypeError 3464: (80-81): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
// TypeError 3464: (85-86): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
