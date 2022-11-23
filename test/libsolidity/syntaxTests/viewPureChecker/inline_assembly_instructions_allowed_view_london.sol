contract C {
    function f() public view {
        assembly {
            pop(difficulty())
        }
    }
}
// ====
// EVMVersion: <=london
// ----
