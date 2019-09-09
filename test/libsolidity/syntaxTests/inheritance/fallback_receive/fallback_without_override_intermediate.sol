contract C {
	fallback() external {}
}
contract D is C {
}
contract E is D {
	fallback() external {}
}
// ----
// TypeError: (78-100): Overriding function is missing 'override' specifier.
