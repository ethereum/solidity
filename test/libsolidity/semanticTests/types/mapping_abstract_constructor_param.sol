abstract contract A {
    constructor (mapping (uint => uint) storage m) {
        m[5] = 20;
    }
}

contract C is A {
    mapping (uint => uint) public m;

    constructor() A(m) {
    }
}
// ====
// compileViaYul: also
// ----
// m(uint256): 1 -> 0
// m(uint256): 5 -> 20
