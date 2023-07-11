contract test {
    function _main(mapping(uint name1 => uint) storage map) internal {
        map[1] = 2;
    }
}
// ----
