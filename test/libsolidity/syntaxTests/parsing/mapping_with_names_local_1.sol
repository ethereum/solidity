contract test {
    mapping(uint name1 => uint name2) map;

    function main() external {
        mapping(uint name3 => uint name4) storage _map = map;
        _map[1] = 2;
    }
}
// ----
