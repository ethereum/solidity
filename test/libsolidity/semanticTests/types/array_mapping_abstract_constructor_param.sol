abstract contract A {
	constructor (mapping (uint => uint) [] storage m) {
		m.push();
		m[0][1] = 2;
	}
}

contract C is A {
	mapping(uint => mapping (uint => uint) []) public m;

	constructor() A(m[1]) {
	}
}
// ----
// m(uint256,uint256,uint256): 0, 0, 0 -> FAILURE
// m(uint256,uint256,uint256): 1, 0, 1 -> 2
// m(uint256,uint256,uint256): 1, 0, 5 -> 0
