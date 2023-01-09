contract test {
    function _main(mapping(uint name1 => mapping(uint name2 => uint name3) name4) storage map) internal {
        map[1][2] = 3;
    }
}
// ----
