contract C {
    mapping(uint => mapping(uint => uint)) public x;
    constructor() {
        x[1][2] = 3;
    }
}
// ====
// compileViaYul: also
// ----
// x(uint256,uint256): 1, 2 -> 3
// x(uint256,uint256): 0, 0 -> 0
