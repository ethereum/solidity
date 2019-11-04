contract C {
	fallback() external {}
}
contract D is C {
	fallback() override external {}
}
