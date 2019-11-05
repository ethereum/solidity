contract C {
	receive() virtual external payable {}
}
contract D is C {
}
contract E is D {
	receive() override external payable {}
}
