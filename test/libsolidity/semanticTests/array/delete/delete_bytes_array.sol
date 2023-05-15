contract C {
    bytes data;

    function f() public returns (uint ret) {
        data.push("a");
        data.push("b");
        delete data;
        assembly {
            ret := sload(data.slot)
        }
    }

    function g() public returns (uint ret) {
        assembly {
            sstore(data.slot, 67)
        }
        data.push("a");
        data.push("b");
        assert(data.length == 35);
        delete data;
        assert(data.length == 0);

        uint size = 999;
        assembly {
            size := sload(data.slot)
            mstore(0, data.slot)
            ret := sload(keccak256(0, 32))
        }
        assert(size == 0);
    }
}
// ----
// f() -> 0
// g() -> 0
