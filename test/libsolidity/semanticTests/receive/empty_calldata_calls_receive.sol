contract A {
    uint public x;
    receive () external payable { ++x; }
}
// ----
// x() -> 0
// ()
// x() -> 1
// (), 1 ether
// x() -> 2
// (): hex"00" -> FAILURE
// (), 1 ether: hex"00" -> FAILURE
