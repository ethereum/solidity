library D { function f(bytes calldata) internal pure {} }
contract C {
	using D for bytes;
	function f(bytes memory _x) public pure {
		_x.f();
	}
}
// ----
// TypeError 9582: (136-140): Member "f" not found or not visible after argument-dependent lookup in bytes memory.
