contract C {
    mapping(uint a => mapping(uint b => uint c)) public x;
    constructor() {
        x[1][2] = 3;
    }
}
// ----
// x(uint256,uint256): 1, 2 -> 3
// x(uint256,uint256): 0, 0 -> 0
