contract c {
    uint spacer1;
    uint spacer2;
    uint[3] data;
    function fill() public {
        for (uint i = 0; i < data.length; ++i) data[i] = i+1;
    }
    function clear() public { delete data; }
}
// ----
// storageEmpty -> 1
// fill() ->
// storageEmpty -> 0
// clear() ->
// storageEmpty -> 1
