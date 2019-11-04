contract C {
	fallback() external {}
}
contract D is C {
}
contract E is D {
	fallback() override external {}
}
