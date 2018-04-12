// Tests that the ConstantEvaluator does not crash for pure non-rational functions.
// Currently it does not evaluate such functions, but this may change in the future
// causing a division by zero error for a.
contract C {
	uint constant a = 1 / (uint(keccak256([0])[0]) - uint(keccak256([0])[0]));
	uint constant b = 1 / uint(keccak256([0]));
	uint constant c = uint(keccak256([0]));
	uint[c] mem;
}
// ----
// TypeError: (392-393): Invalid array length, expected integer literal or constant expression.
