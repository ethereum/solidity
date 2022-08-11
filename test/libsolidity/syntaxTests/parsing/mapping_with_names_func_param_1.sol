contract test {
    function _main(mapping(uint name1 => uint name2) storage map) internal {
        map[1] = 2;
    }
}
// ----
