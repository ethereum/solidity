contract C {
    function f() public view returns(bytes32) {
        return blobhash(0);
    }
    function g() public view returns(bytes32) {
        return blobhash(1);
    }
    function h() public view returns(bytes32) {
        // NOTE: blobhash(2) should return 0 since EVMHost has only two blob hashes injected in the block the transaction is being executed.
        return blobhash(2);
    }
}
// ====
// EVMVersion: >=cancun
// ----
// f() -> 0x0100000000000000000000000000000000000000000000000000000000000001
// g() -> 0x0100000000000000000000000000000000000000000000000000000000000002
// h() -> 0x00
