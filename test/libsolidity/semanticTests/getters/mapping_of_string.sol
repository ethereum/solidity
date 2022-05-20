contract C {
    mapping(string => uint8[3]) public x;
    constructor() {
        x["abc"][0] = 1;
        x["abc"][2] = 3;
        x["abc"][1] = 2;
        x["def"][1] = 9;
    }
}
// ----
// x(string,uint256): 0x40, 0, 3, "abc" -> 1
// x(string,uint256): 0x40, 1, 3, "abc" -> 2
// x(string,uint256): 0x40, 2, 3, "abc" -> 3
// x(string,uint256): 0x40, 0, 3, "def" -> 0x00
// x(string,uint256): 0x40, 1, 3, "def" -> 9
// x(string,uint256): 0x40, 2, 3, "def" -> 0x00
