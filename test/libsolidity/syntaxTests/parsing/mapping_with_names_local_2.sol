contract test {
    mapping(uint name1 => uint) map;

    function main() external {
        mapping(uint => uint name4) storage _map = map;
        _map[1] = 2;
    }
}
// ----
