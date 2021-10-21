contract C {
    function() internal f = g;
    uint8 public a = 20;

    function g() internal {
    }
    function slots() public returns (uint _a, uint _f) {
        assembly {
            _a := a.slot
            _f := f.slot
        }
    }
    function offsets() public returns (uint _a, uint _f) {
        assembly {
            _a := a.offset
            _f := f.offset
        }
    }
    function delete_a() external {
        delete a;
    }
    // The actual value cannot be returned since it would be different for various settings
    // (optimized v/s non-optimized, legacy v/s IR).
    function test_f_non_zero() external returns (bool) {
        uint storage_value;
        assembly {
            storage_value := sload(f.slot)
        }
        assert(storage_value != 0);
        return true;
    }
}
// ====
// compileViaYul: also
// ----
// a() -> 0x14
// slots() -> 0, 0
// offsets() -> 8, 0
// delete_a() ->
// test_f_non_zero() -> true
