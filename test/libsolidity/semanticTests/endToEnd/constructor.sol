contract test {
    mapping(uint => uint) data;
    constructor() public {
        data[7] = 8;
    }

    function get(uint key) public returns(uint value) {
        return data[key];
    }
}

// ====
// compileViaYul: also
// ----
// get(uint256): 0x6 -> 0x0
// get(uint256): 0x7 -> 0x8
