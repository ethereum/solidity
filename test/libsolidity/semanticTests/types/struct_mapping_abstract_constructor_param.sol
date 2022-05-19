struct S {
	mapping (uint => uint) m;
}

abstract contract A {
	constructor (S storage s) {
		s.m[5] = 16;
	}
}

contract C is A {
	mapping(uint => S) m;

	constructor() A(m[1]) {
	}

	function getM(uint a, uint b) external returns (uint) {
		return m[a].m[b];
	}
}
// ----
// getM(uint256,uint256): 0, 0 -> 0
// getM(uint256,uint256): 1, 5 -> 0x10
// getM(uint256,uint256): 1, 0 -> 0
