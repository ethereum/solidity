contract C {
    function f() public view {
        assembly { pop(chainid()) }
    }
    function g() public view returns (uint) {
        return block.chainid;
    }
}
// ====
// EVMVersion: >=istanbul
// ----
