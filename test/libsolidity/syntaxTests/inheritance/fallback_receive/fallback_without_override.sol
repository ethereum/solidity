contract C {
	fallback() external {}
}
contract D is C {
	fallback() external {}
}
// ----
// TypeError: (58-80): Overriding function is missing 'override' specifier.
