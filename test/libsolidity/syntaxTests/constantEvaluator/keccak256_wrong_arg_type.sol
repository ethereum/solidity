contract C {
	bytes32 constant x = "abcd1234";
	uint[keccak256(42)] a;
	uint[keccak256(x)] b;
}
// ----
// TypeError 5462: (53-66): Invalid array length, expected integer literal or constant expression.
// TypeError 5462: (77-89): Invalid array length, expected integer literal or constant expression.
