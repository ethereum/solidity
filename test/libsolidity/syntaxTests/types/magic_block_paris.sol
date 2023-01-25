contract C {
    function f() public view returns (uint) {
        return block.prevrandao;
    }
    function g() public view returns (uint ret) {
        assembly {
            ret := prevrandao()
        }
    }
}
// ====
// EVMVersion: >=paris
// ----
