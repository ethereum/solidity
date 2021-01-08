
contract c {
    function set() public returns (bool) { data1 = msg.data; return true; }
    function reset() public returns (bool) { data1 = data2; return true; }
    bytes data1;
    bytes data2;
}
// ====
// compileViaYul: also
// ----
// set(): 1, 2, 3, 4, 5 -> true
// storage_empty -> false
// reset() -> true
// storage_empty -> true
