contract C {
    uint[] storageArray;
    function popEmpty() public {
        storageArray.pop();
    }
}
// ====
// EVMVersion: >=petersburg
// compileViaYul: also
// ----
// popEmpty() -> FAILURE
