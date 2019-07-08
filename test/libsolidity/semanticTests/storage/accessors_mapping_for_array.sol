contract test {
    mapping(uint => uint[8]) public data;
    mapping(uint => uint[]) public dynamicData;
    constructor() public {
        data[2][2] = 8;
        dynamicData[2].length = 3;
        dynamicData[2][2] = 8;
    }
}
// ----
// data(uint256,uint256): 2, 2 -> 8
// data(uint256,uint256): 2, 8 -> FAILURE  # NB: the original code contained a bug here #
// dynamicData(uint256,uint256): 2, 2 -> 8
// dynamicData(uint256,uint256): 2, 8 -> FAILURE
