contract C {
    function f() public view {
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
