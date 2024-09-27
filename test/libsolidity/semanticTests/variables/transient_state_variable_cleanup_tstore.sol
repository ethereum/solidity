contract C {
    uint8 transient x;
    function f() public returns(uint256 r) {
        assembly {
            tstore(x.slot, 0xFFFF)
        }
       return x;

    }
}
// ====
// EVMVersion: >=cancun
// ----
// f() -> 0xff
