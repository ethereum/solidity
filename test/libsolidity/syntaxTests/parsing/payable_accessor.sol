contract test {
	uint payable x;
}
// ----
// ParserError 9106: (22-29='payable'): State mutability can only be specified for address types.
