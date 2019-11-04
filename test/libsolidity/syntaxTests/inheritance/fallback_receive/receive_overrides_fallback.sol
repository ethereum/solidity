contract C {
	fallback() external {}
}
contract D is C {
	receive() override external payable {}
}
// ----
// TypeError: (68-76): Function has override specified but does not override anything.
