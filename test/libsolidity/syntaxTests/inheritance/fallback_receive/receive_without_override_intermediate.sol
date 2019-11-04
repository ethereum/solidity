contract C {
	receive() external payable {}
}
contract D is C {
}
contract E is D {
	receive() external payable {}
}
// ----
// TypeError: (85-114): Overriding function is missing 'override' specifier.
