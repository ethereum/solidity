contract C {
	uint[keccak256()] a;
	uint[keccak256("a", "b")] b;
}
// ----
// TypeError 8392: (19-30): keccak256 function expects 1 parameter, but 0 were given.
// TypeError 5462: (19-30): Invalid array length, expected integer literal or constant expression.
// TypeError 8392: (41-60): keccak256 function expects 1 parameter, but 2 were given.
// TypeError 5462: (41-60): Invalid array length, expected integer literal or constant expression.
