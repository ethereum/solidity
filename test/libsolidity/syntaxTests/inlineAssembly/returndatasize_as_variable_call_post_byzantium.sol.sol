contract C {
    function f() public pure {
        uint returndatasize;
        returndatasize;
        assembly {
            let x := returndatasize()
        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
