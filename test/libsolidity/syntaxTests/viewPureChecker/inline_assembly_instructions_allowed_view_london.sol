contract C {
    function f() public view {
        assembly {
            // Renamed in paris
            pop(difficulty())
        }
    }
}
// ====
// EVMVersion: =london
// ----
