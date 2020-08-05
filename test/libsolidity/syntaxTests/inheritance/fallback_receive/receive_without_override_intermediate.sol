contract C {
	receive() virtual external payable {}
}
contract D is C {
}
contract E is D {
	receive() external payable {}
}
// ----
// TypeError 9456: (93-122): Overriding function is missing "override" specifier.
