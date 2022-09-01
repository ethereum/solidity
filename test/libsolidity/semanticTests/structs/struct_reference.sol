contract test {
    struct s2 {
        uint32 z;
        mapping(uint8 => s2) recursive;
    }
    s2 data;
    function check() public returns (bool ok) {
        return data.z == 2 &&
            data.recursive[0].z == 3 &&
            data.recursive[0].recursive[1].z == 0 &&
            data.recursive[0].recursive[0].z == 1;
    }
    function set() public {
        data.z = 2;
        mapping(uint8 => s2) storage map = data.recursive;
        s2 storage inner = map[0];
        inner.z = 3;
        inner.recursive[0].z = inner.recursive[1].z + 1;
    }
}
// ----
// check() -> false
// set() ->
// check() -> true
