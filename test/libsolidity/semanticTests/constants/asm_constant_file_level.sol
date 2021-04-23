address constant e = 0x1212121212121212121212121000002134593163;

contract C {
  function f() public returns (address z) {
    assembly { z := e }
  }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f() -> 0x1212121212121212121212121000002134593163
