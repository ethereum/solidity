contract c {
    fallback() external { data = msg.data; }
    function del() public returns (bool) { delete data; return true; }
    bytes data;
}
// ====
// compileViaYul: also
// ----
// (): 7 ->
// storage: nonempty
// del(): 7 -> true
// storage: empty
