contract C {
    // ecrecover should return zero for malformed input
	// (v should be 27 or 28, not 1)
	// Note that the precompile does not return zero but returns nothing.
    function f() public returns (address) {
        return ecrecover(bytes32(uint(-1)), 1, bytes32(uint(2)), bytes32(uint(3)));
    }
}
// ----
// f() -> 0
