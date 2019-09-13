contract C {
	uint[] array;

	function f() public returns (uint) {
		uint y = array.push();
		return y;
	}

	function lv(uint value) public {
		array.push() = value;
	}

	function a(uint index) public view returns (uint) {
		return array[index];
	}

	function l() public view returns (uint) {
		return array.length;
	}

}
// ----
// l() -> 0
// lv(uint256): 42 ->
// l() -> 1
// a(uint256): 0 -> 42
// f() -> 0
// l() -> 2
// a(uint256): 1 -> 0
// lv(uint256): 111 ->
// l() -> 3
// a(uint256): 2 -> 111
