contract test {
    struct Data { uint a; uint8 b; mapping(uint => uint) c; bool d; }
    mapping(uint => Data) public data;
    constructor() public {
        data[7].a = 1;
        data[7].b = 2;
        data[7].c[0] = 3;
        data[7].d = true;
    }
}
// ----
// data(uint256): 7 -> 1, 2, true
