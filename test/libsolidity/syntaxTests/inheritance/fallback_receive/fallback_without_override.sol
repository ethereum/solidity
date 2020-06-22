contract C {
	fallback() virtual external {}
}
contract D is C {
	fallback() external {}
}
// ----
// TypeError 9456: (66-88): Overriding function is missing "override" specifier.
