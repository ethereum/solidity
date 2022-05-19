library Lib {
    function set(mapping(uint => uint) storage m, uint key, uint value) internal
    {
        m[key] = value;
    }
    function get(mapping(uint => uint) storage m, uint key) internal view returns (uint)
    {
        return m[key];
    }
}
contract Test {
    mapping(uint => uint) m;
    function set(uint256 key, uint256 value) public returns (uint)
    {
        uint oldValue = Lib.get(m, key);
        Lib.set(m, key, value);
        return oldValue;
    }
    function get(uint256 key) public view returns (uint) {
        return Lib.get(m, key);
    }
}
// ====
// compileToEwasm: false
// ----
// library: Lib
// set(uint256,uint256): 1, 42 -> 0
// set(uint256,uint256): 2, 84 -> 0
// set(uint256,uint256): 21, 7 -> 0
// get(uint256): 0 -> 0
// get(uint256): 1 -> 0x2a
// get(uint256): 2 -> 0x54
// get(uint256): 21 -> 7
// set(uint256,uint256): 1, 21 -> 0x2a
// set(uint256,uint256): 2, 42 -> 0x54
// set(uint256,uint256): 21, 14 -> 7
// get(uint256): 0 -> 0
// get(uint256): 1 -> 0x15
// get(uint256): 2 -> 0x2a
// get(uint256): 21 -> 14
