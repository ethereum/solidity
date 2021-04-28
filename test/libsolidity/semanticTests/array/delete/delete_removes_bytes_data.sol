contract c {
    fallback() external { data = msg.data; }
    function del() public returns (bool) { delete data; return true; }
    bytes data;
}
// ====
// compileViaYul: also
// ----
// (): 7 ->
// storageEmpty -> 0
// del(): 7 -> true
// storageEmpty -> 1
