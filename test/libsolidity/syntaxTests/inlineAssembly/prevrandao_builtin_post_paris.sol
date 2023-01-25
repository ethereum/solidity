contract C {
    function f() public view returns (uint256) {
        return block.prevrandao;
    }

    function g() public view returns (uint256 ret) {
        assembly {
            ret := prevrandao()
        }
    }
}
// ====
// EVMVersion: >=paris
// ----
