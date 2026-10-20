// The test only runs on EVM versions that satisfy all of the comma-separated constraints.
// `chainid` is not available before istanbul and `difficulty` is not available starting from paris.
contract C {
    function f() public view returns (uint a, uint b) {
        assembly {
            a := chainid()
            b := difficulty()
        }
    }
}
// ====
// EVMVersion: >=istanbul,<paris
// ----
