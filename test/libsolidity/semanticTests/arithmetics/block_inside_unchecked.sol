contract C {
  function f() public returns (uint y) {
    unchecked{{
        uint max = type(uint).max;
        uint x = max + 1;
        y = x;
    }}
  }
}
// ====
// compileToEwasm: also
// ----
// f() -> 0x00
