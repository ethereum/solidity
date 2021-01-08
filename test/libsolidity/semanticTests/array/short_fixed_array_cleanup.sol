contract c {
    uint spacer1;
    uint spacer2;
    uint[3] data;
    function fill() public {
        for (uint i = 0; i < data.length; ++i) data[i] = i+1;
    }
    function clear() public { delete data; }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// storage_empty -> true
// fill() ->
// storage_empty -> false
// clear() ->
// storage_empty -> true
