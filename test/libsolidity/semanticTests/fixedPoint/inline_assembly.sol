contract A {
  function s128x18() public pure returns (fixed x) {
    assembly {x := 1000000111000222000333}
  }

  function s128x16() public pure returns (fixed128x16 x) {
    assembly {x := 1000000111000222000333}
  }

  function s128x8() public pure returns (fixed128x8 x) {
    assembly {x := 1000000111000222000333}
  }

  function s128x4() public pure returns (fixed128x4 x) {
    assembly {x := 1000000111000222000333}
  }

  function s128x2() public pure returns (fixed128x2 x) {
    assembly {x := 1000000111000222000333}
  }

  function s128x0() public pure returns (fixed128x0 x) {
    assembly {x := 1000000111000222000333}
  }

  function s128x0_2() public pure returns (fixed128x0 x, fixed128x2 y) {
    assembly {
        x := 1000000111000222000333
        y := 1000000111000222000333
    }
  }

  function s128x2(fixed128x2 i) public pure returns (fixed128x2 x) {
    assembly {x := i}
  }

  function u128x2(ufixed128x2 i) public pure returns (ufixed128x2 x) {
    assembly {x := i}
  }

  function u128x0(ufixed128x0 i) public pure returns (ufixed128x0 x) {
    assembly {x := i}
  }

  function s128x2_add(fixed128x2 a, fixed128x2 b) public pure returns (fixed128x2 x) {
    assembly {x := add(a, b)}
  }

  function s128x0(fixed128x0 i) public pure returns (fixed128x0 x) {
    assembly {x := i}
  }

  function s32x0(fixed32x0 i) public pure returns (fixed32x0 x) {
    assembly {x := i}
  }

  function s16x0(fixed16x0 i) public pure returns (fixed16x0 x) {
    assembly {x := i}
  }

  function s8x0(fixed8x0 i) public pure returns (fixed8x0 x) {
    assembly {x := i}
  }

  function u32x0(ufixed32x0 i) public pure returns (ufixed32x0 x) {
    assembly {x := i}
  }

  function u16x0(ufixed16x0 i) public pure returns (ufixed16x0 x) {
    assembly {x := i}
  }

  function u8x0(ufixed8x0 i) public pure returns (ufixed8x0 x) {
    assembly {x := i}
  }

  function u32x2(ufixed32x2 i) public pure returns (ufixed32x2 x) {
    assembly {x := i}
  }

  function u16x2(ufixed16x2 i) public pure returns (ufixed16x2 x) {
    assembly {x := i}
  }

  function u8x2(ufixed8x2 i) public pure returns (ufixed8x2 x) {
    assembly {x := i}
  }

  function u32168x2(ufixed32x2 a, ufixed16x2 b, ufixed8x2 c) public pure returns (ufixed32x2 x) {
    assembly {x := a}
  }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// u128x0(ufixed128x0): 123 -> 123
// u128x2(ufixed128x2): 1.23 -> 1.23
// s128x2(fixed128x2): -2.34 -> -2.34
// s128x2(fixed128x2): 2.34 -> 2.34
// u128x0(ufixed128x0): 123 -> 123
// s128x0(fixed128x0): -234 -> -234
// s128x2_add(fixed128x2,fixed128x2): -1.23, -2.34 -> -3.57
// s128x2_add(fixed128x2,fixed128x2): -1.23, 2.34 -> 1.11
// s128x2_add(fixed128x2,fixed128x2): 1.23, -2.34 -> -1.11
// s128x2_add(fixed128x2,fixed128x2): 1.23, 2.34 -> 3.57
// s128x2_add(fixed128x2,fixed128x2): 1701411834604692317316873037158841057.27, -1701411834604692317316873037158841057.28 -> -0.01
// u32x0(ufixed32x0): 0 -> 0
// u16x0(ufixed16x0): 0 -> 0
// u8x0(ufixed8x0): 0 -> 0
// u32x0(ufixed32x0): 4294967295 -> 4294967295
// u16x0(ufixed16x0): 65535 -> 65535
// u8x0(ufixed8x0): 255 -> 255
// u32x2(ufixed32x2): 42949672.95 -> 42949672.95
// u16x2(ufixed16x2): 655.35 -> 655.35
// u8x2(ufixed8x2): 2.55 -> 2.55
// u32168x2(ufixed32x2,ufixed16x2,ufixed8x2): 42949672.95, 655.35, 2.55 -> 42949672.95
// s128x18() -> 1000.000111000222000333
// s128x16() -> 100000.0111000222000333
// s128x8() -> 10000001110002.22000333
// s128x4() -> 100000011100022200.0333
// s128x2() -> 10000001110002220003.33
// s128x0() -> 1000000111000222000333
// s128x0_2() -> 1000000111000222000333, 10000001110002220003.33
