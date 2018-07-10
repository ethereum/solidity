contract C {
	function f() public {
		uint a = (1);
		(uint b,) = 1;
		(uint c, uint d) = (1, 2 + a);
		(uint e,) = (1, 2, b);
		(a) = 3;
	}
}
// ----
// Warning: (54-67): Different number of components on the left hand side (2) than on the right hand side (1).
// Warning: (104-125): Different number of components on the left hand side (2) than on the right hand side (3).
// Warning: (72-78): Unused local variable.
// Warning: (80-86): Unused local variable.
// Warning: (105-111): Unused local variable.
// Warning: (14-140): Function state mutability can be restricted to pure
