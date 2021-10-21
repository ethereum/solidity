contract C {
    uint8 public a = 20;
    function() external public f = this.g;
    function g() external {
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
}
// ====
// compileViaYul: also
// ----
// a() -> 0x14
// f() -> 7175878113405833249322534082293024008058894263831758935649259662147697246208
// slots() -> 0, 0
// offsets() -> 0, 1
