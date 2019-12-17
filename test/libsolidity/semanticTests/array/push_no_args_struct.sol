contract C {
	struct S {
		uint x;
	}

	S[] array;

	function f(uint y) public {
		S storage s = array.push();
		g(s, y);
	}

	function g(S storage s, uint y) internal {
		s.x = y;
	}

	function h(uint y) public {
		g(array.push(), y);
	}

	function lv(uint y) public {
		array.push().x = y;
	}

	function a(uint i) public returns (uint) {
		return array[i].x;
	}

	function l() public returns (uint) {
		return array.length;
	}

}
// ----
// l() -> 0
// f(uint256): 42 ->
// l() -> 1
// a(uint256): 0 -> 42
// h(uint256): 84 ->
// l() -> 2
// a(uint256): 1 -> 84
// lv(uint256): 4096 ->
// l() -> 3
// a(uint256): 2 -> 4096
