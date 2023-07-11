contract Base {
	uint public x;
	uint public y;
	function init(uint a, uint b) public {
		x = a;
		y = b;
	}
	function init(uint a) public {
		x = a;
	}
}

contract Child is Base {
	function cInit(uint c) public {
		Base.init(c);
	}
	function cInit(uint c, uint d) public {
		Base.init(c, d);
	}
}
// ----
// x() -> 0
// y() -> 0
// cInit(uint256): 2 ->
// x() -> 2
// y() -> 0
// cInit(uint256,uint256): 3, 3 ->
// x() -> 3
// y() -> 3
