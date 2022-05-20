contract C {
    function uf(bool b, uint x, uint y) public pure returns (uint) {
        require(b, "failure");
        return x - y;
    }
    function onlyPanic(bool b, uint x, uint y) public returns (uint r, uint code) {
        try this.uf(b, x, y) returns (uint b) {
            r = b;
        } catch Panic(uint c) {
            code = c;
        }
    }
    function panicAndError(bool b, uint x, uint y) public returns (uint r, uint code, string memory msg_) {
        try this.uf(b, x, y) returns (uint b) {
            r = b;
        } catch Panic(uint c) {
            code = c;
        } catch Error(string memory _errmsg) {
            msg_ = _errmsg;
        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// onlyPanic(bool,uint256,uint256): true, 7, 6 -> 1, 0x00
// onlyPanic(bool,uint256,uint256): true, 6, 7 -> 0x00, 0x11
// onlyPanic(bool,uint256,uint256): false, 7, 6 -> FAILURE, hex"08c379a0", 0x20, 7, "failure"
// onlyPanic(bool,uint256,uint256): false, 6, 7 -> FAILURE, hex"08c379a0", 0x20, 7, "failure"
// panicAndError(bool,uint256,uint256): true, 7, 6 -> 1, 0x00, 0x60, 0x00
// panicAndError(bool,uint256,uint256): true, 6, 7 -> 0x00, 0x11, 0x60, 0x00
// panicAndError(bool,uint256,uint256): false, 7, 6 -> 0x00, 0x00, 0x60, 7, "failure"
// panicAndError(bool,uint256,uint256): false, 6, 7 -> 0x00, 0x00, 0x60, 7, "failure"
