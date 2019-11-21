contract test {
    uint[8] public data;
    uint[] public dynamicData;
    uint24[] public smallTypeData;
    struct st { uint a; uint[] finalArray; }
    mapping(uint256 => mapping(uint256 => st[5])) public multiple_map;

    constructor() public {
        data[0] = 8;

        dynamicData.push();
        dynamicData.push();
        dynamicData.push(8);

        smallTypeData = new uint24[](128);
        smallTypeData[1] = 22;
        smallTypeData[127] = 2;

        multiple_map[2][1][2].a = 3;
        for (uint i = 0; i < 4; i++)
            multiple_map[2][1][2].finalArray.push();
        multiple_map[2][1][2].finalArray[3] = 5;
    }
}
// ----
// data(uint256): 0 -> 8
// data(uint256): 8 -> FAILURE
// dynamicData(uint256): 2 -> 8
// dynamicData(uint256): 8 -> FAILURE
// smallTypeData(uint256): 1 -> 22
// smallTypeData(uint256): 127 -> 2
// smallTypeData(uint256): 128 -> FAILURE
// multiple_map(uint256,uint256,uint256): 2, 1, 2 -> 3
