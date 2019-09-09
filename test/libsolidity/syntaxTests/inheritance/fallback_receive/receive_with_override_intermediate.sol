contract C {
	receive() external payable {}
}
contract D is C {
}
contract E is D {
	receive() override external payable {}
}
