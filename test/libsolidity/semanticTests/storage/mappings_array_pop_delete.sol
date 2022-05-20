contract C {
	mapping (uint => uint)[] a;

	function n1(uint key, uint value) public {
		a.push();
		a[a.length - 1][key] = value;
	}

	function n2() public {
		a.push();
	}

	function map(uint key) public view returns (uint) {
		return a[a.length - 1][key];
	}

	function p() public {
		a.pop();
	}

	function d() public returns (uint) {
		delete a;
		return a.length;
	}
}
// ----
// n1(uint256,uint256): 42, 64 ->
// map(uint256): 42 -> 64
// p() ->
// n2() ->
// map(uint256): 42 -> 64
// d() -> 0
// n2() ->
// map(uint256): 42 -> 64
