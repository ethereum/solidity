contract C {
	receive() virtual external payable {}
}
contract D is C {
	receive() external payable {}
}
// ----
// TypeError: (73-102): Overriding function is missing 'override' specifier.
