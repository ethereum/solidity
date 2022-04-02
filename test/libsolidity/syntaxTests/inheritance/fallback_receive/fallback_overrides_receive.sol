contract C {
	receive() external payable {}
}
contract D is C {
	fallback() override external {}
}
// ----
// TypeError 7792: (76-84='override'): Function has override specified but does not override anything.
