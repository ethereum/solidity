contract test {
    mapping(uint name1 => mapping(uint name2 => mapping(uint name3 => uint name4) name5) name6) map;

    function main() external {
        mapping(uint name1 => mapping(uint name2 => mapping(uint name3 => uint name4) name5) name6) storage _map = map;
        _map[1][2][3] = 4;
    }
}
// ----
