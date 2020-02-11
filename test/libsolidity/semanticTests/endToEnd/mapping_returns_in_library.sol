library Lib {
    function choose_mapping(mapping(uint => uint) storage a, mapping(uint => uint) storage b, bool c) internal pure returns(mapping(uint => uint) storage) {
        return c ? a : b;
    }
}
contract Test {
    mapping(uint => uint) a;
    mapping(uint => uint) b;

    function set(bool choice, uint256 key, uint256 value) public returns(uint) {
        mapping(uint => uint) storage m = Lib.choose_mapping(a, b, choice);
        uint oldValue = m[key];
        m[key] = value;
        return oldValue;
    }

    function get(bool choice, uint256 key) public view returns(uint) {
        return Lib.choose_mapping(a, b, choice)[key];
    }

    function get_a(uint256 key) public view returns(uint) {
        return a[key];
    }

    function get_b(uint256 key) public view returns(uint) {
        return b[key];
    }
}

// ----
// set(bool,uint256,uint256): 1, 1, 42 -> 0
// set(bool,uint256,uint256): 1, 2, 84 -> 0
// set(bool,uint256,uint256): 1, 21, 7 -> 0
// set(bool,uint256,uint256): 0, 1, 10 -> 0
// set(bool,uint256,uint256): 0, 2, 11 -> 0
// set(bool,uint256,uint256): 0, 21, 12 -> 0
// get(bool,uint256): 1, 0 -> 0
// get(bool,uint256): 1, 1 -> 42
// get(bool,uint256): 1, 2 -> 84
// get(bool,uint256): 1, 21 -> 7
// get_a(uint256): 0 -> 0
// get_a(uint256): 1 -> 42
// get_a(uint256): 2 -> 84
// get_a(uint256): 21 -> 7
// get(bool,uint256): 0, 0 -> 0
// get(bool,uint256): 0, 1 -> 10
// get(bool,uint256): 0, 2 -> 11
// get(bool,uint256): 0, 21 -> 12
// get_b(uint256): 0 -> 0
// get_b(uint256): 1 -> 10
// get_b(uint256): 2 -> 11
// get_b(uint256): 21 -> 12
// set(bool,uint256,uint256): 1, 1, 21 -> 42
// set(bool,uint256,uint256): 1, 2, 42 -> 84
// set(bool,uint256,uint256): 1, 21, 14 -> 7
// set(bool,uint256,uint256): 0, 1, 30 -> 10
// set(bool,uint256,uint256): 0, 2, 31 -> 11
// set(bool,uint256,uint256): 0, 21, 32 -> 12
// get_a(uint256): 0 -> 0
// get_a(uint256): 1 -> 21
// get_a(uint256): 2 -> 42
// get_a(uint256): 21 -> 14
// get(bool,uint256): 1, 0 -> 0
// get(bool,uint256): 1, 1 -> 21
// get(bool,uint256): 1, 2 -> 42
// get(bool,uint256): 1, 21 -> 14
// get_b(uint256): 0 -> 0
// get_b(uint256): 1 -> 30
// get_b(uint256): 2 -> 31
// get_b(uint256): 21 -> 32
// get(bool,uint256): 0, 0 -> 0
// get(bool,uint256): 0, 1 -> 30
// get(bool,uint256): 0, 2 -> 31
// get(bool,uint256): 0, 21 -> 32
