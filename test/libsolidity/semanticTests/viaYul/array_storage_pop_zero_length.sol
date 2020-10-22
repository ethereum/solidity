contract C {
    uint[] storageArray;
    function popEmpty() public {
        storageArray.pop();
    }
}
// ====
// EVMVersion: >=petersburg
// compileViaYul: true
// ----
// popEmpty() -> FAILURE, hex"4e487b71", 0x31
