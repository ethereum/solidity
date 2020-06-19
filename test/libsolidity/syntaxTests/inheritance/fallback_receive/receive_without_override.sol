contract C {
	receive() virtual external payable {}
}
contract D is C {
	receive() external payable {}
}
// ----
// TypeError 9456: (73-102): Overriding function is missing "override" specifier.
