contract C {
    receive () payable external { }
}
// ====
// compileViaYul: also
// ----
// (), 1 ether
// (), 1 ether: 1 -> FAILURE
