contract test {
    mapping(uint name1 => uint[] name4) map;

    function main() external {
        mapping(uint name5 => uint[] name6) storage _map = map;
        _map[1].push(2);
    }
}
// ----
