contract C {
    function f() public {
        assembly {
            pop(difficulty())
        }
    }
}
// ====
// EVMVersion: <=london
// ----
// Warning 2018: (17-103): Function state mutability can be restricted to view
