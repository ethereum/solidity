contract C {
    function zero() external returns (uint) {
        return 0;
    }

}
// ====
// compileViaYul: also
// EVMVersion: >=shanghai
// ----
// zero() -> 0
