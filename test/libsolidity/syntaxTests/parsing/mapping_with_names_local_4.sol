contract test {
    mapping(uint name1 => mapping(uint name2 => uint name3) name4) map;

    function main() external {
        mapping(uint name5 => uint name6) storage _map = map[1];
        _map[1] = 2;
    }
}
// ----
