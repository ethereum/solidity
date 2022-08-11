contract test {
    function _main(mapping(uint name1 => mapping(uint name2 => mapping(uint name3 => uint name4) name5) name6) storage map) internal {
        map[1][2][3] = 4;
    }
}
// ----
