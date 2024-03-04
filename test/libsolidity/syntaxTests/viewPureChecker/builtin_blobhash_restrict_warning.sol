contract C {
    function g() public {
        bytes32 w = blobhash(0);
        w;
    }
}
// ====
// EVMVersion: >=cancun
// ----
// Warning 2018: (17-88): Function state mutability can be restricted to view
