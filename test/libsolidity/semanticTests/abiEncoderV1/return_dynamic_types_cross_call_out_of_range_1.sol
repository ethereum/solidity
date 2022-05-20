contract C {
    function dyn(uint x) public returns (bytes memory a) {
        assembly {
            mstore(0, 0x20)
            mstore(0x20, 0x21)
            return(0, x)
        }
    }
    function f(uint x) public returns (bool) {
        this.dyn(x);
        return true;
    }
}
// ====
// EVMVersion: =homestead
// ----
// f(uint256): 0x60 -> true
// f(uint256): 0x7f -> true
// f(uint256): 0x80 -> true