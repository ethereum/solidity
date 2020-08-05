library L {
    function at(mapping(uint => uint) storage a, uint i) internal view returns (uint) {
        return a[i];
    }
}

contract C {
    using L for mapping(uint => uint);

    mapping(uint => uint) map;

    function mapValue(uint a) public returns (uint) {
        map[42] = 0x24;
        map[66] = 0x66;

        return map.at(a);
    }
}
// ====
// compileViaYul: also
// ----
// mapValue(uint256): 42 -> 0x24
