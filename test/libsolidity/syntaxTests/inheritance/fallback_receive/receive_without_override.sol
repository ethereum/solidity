contract C {
	receive() external payable {}
}
contract D is C {
	receive() external payable {}
}
// ----
// TypeError: (65-94): Overriding function is missing 'override' specifier.
