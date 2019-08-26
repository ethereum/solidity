contract BaseBase {
	uint public x;
	uint public y;
	function init(uint a, uint b) public {
		x = b;
		y = a;
	}
	function init(uint a) public {
		x = a + 1;
	}
}

contract Base is BaseBase {
	function init(uint a, uint b) public override {
		x = a;
		y = b;
	}
	function init(uint a) public override {
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
	function bInit(uint c) public {
		BaseBase.init(c);
	}
	function bInit(uint c, uint d) public {
		BaseBase.init(c, d);
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
// bInit(uint256): 4 ->
// x() -> 5
// bInit(uint256,uint256): 9, 10 ->
// x() -> 10
// y() -> 9
