contract C {
	uint[] r;
    function f() internal view returns (uint[] storage s) {
        assembly { pop(s_slot) }
        s = r;
    }
}
// ----
// TypeError: (92-126): This variable is of storage pointer type and can be accessed without prior assignment.
