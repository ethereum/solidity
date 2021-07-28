contract A {
  function f128x18() public pure returns (fixed x) {
    assembly { x := 1000000111000222000333 }
  }
  function f128x16() public pure returns (fixed128x16 x) {
    assembly { x := 1000000111000222000333 }
  }
  function f128x8() public pure returns (fixed128x8 x) {
    assembly { x := 1000000111000222000333 }
  }
  function f128x4() public pure returns (fixed128x4 x) {
    assembly { x := 1000000111000222000333 }
  }
  function f128x2() public pure returns (fixed128x2 x) {
    assembly { x := 1000000111000222000333 }
  }
  function f128x0() public pure returns (fixed128x0 x) {
    assembly { x := 1000000111000222000333 }
  }
  function i128x2(fixed128x2 i) public pure returns (fixed128x2 x) {
    assembly { x := i }
  }
  function add128x2(fixed128x2 a, fixed128x2 b) public pure returns (fixed128x2 x) {
    assembly { x := add(a, b) }
  }
  function i128x0(fixed128x0 i) public pure returns (fixed128x0 x) {
    assembly { x := i }
  }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f128x18() -> 1000.000111000222000333
// f128x16() -> 100000.0111000222000333
// f128x8() -> 10000001110002.22000333
// f128x4() -> 100000011100022200.0333
// f128x2() -> 10000001110002220003.33
// f128x0() -> 1000000111000222000333
// i128x0(fixed128x0): 1000000111000222000333 -> 1000000111000222000333
// i128x2(fixed128x2): 10000001110002220003.33 -> 10000001110002220003.33
// add128x2(fixed128x2,fixed128x2): 1.23, 2.34 -> 3.57
