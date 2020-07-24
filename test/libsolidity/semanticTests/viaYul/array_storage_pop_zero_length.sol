contract C {
    uint[] storageArray;
    function popEmpty() public {
        storageArray.pop();
    }
}
// ====
// EVMVersion: >=petersburg
// compileToEwasm: also
// compileViaYul: true
// ----
// popEmpty() -> FAILURE
