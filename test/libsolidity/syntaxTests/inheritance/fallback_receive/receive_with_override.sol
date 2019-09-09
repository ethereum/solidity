contract C {
	receive() external payable {}
}
contract D is C {
	receive() override external payable {}
}
