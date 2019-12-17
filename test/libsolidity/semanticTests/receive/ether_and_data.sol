contract C {
    receive () payable external { }
}
// ----
// (), 1 ether
// (), 1 ether: 1 -> FAILURE
