contract C {
	receive() virtual external payable {}
}
contract D is C {
	receive() override external payable {}
}
