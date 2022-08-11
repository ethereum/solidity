contract test {
    mapping(uint => uint name2) map;

    function main() external {
        mapping(uint name3 => uint) storage _map = map;
        _map[1] = 2;
    }
}
// ----
