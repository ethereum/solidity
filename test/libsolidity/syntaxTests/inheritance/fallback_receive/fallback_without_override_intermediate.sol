contract C {
	fallback() virtual external {}
}
contract D is C {
}
contract E is D {
	fallback() external {}
}
// ----
// TypeError: (86-108): Overriding function is missing 'override' specifier.
