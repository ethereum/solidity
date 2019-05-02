contract C {
    mapping(uint => uint) simple;
    mapping(uint16 => uint) cleanup;
    mapping(string => uint) str;
    mapping(uint => mapping(uint => uint)) twodim;
    function test_simple(uint _off) public returns (uint _a, uint _b, uint _c) {
        simple[_off + 2] = 3;
        simple[_off + 3] = 4;
        simple[uint(-1)] = 5;
        _c = simple[uint(-1)];
        _b = simple[3 + _off];
        _a = simple[2 + _off];
    }
    function test_cleanup() public returns (bool) {
        uint16 x;
        assembly { x := 0xffff0001 }
        cleanup[x] = 3;
        return cleanup[1] == 3;
    }
    function test_str() public returns (bool) {
        str["abc"] = 3;
        string memory s = "abc";
        return str[s] == 3;
    }
    function test_twodim() public returns (uint a, uint b) {
        twodim[2][3] = 3;
        a = twodim[3][2];
        b = twodim[2][3];
    }
}
// ====
// compileViaYul: true
// ----
// test_simple(uint256): 0 -> 3, 4, 5
// test_simple(uint256): 1 -> 3, 4, 5
// test_simple(uint256): 2 -> 3, 4, 5
// test_cleanup() -> true
// test_str() -> true
// test_twodim() -> 0, 3
