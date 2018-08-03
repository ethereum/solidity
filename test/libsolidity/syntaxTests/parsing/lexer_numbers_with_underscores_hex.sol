contract C {
  function f() public pure {
    uint x1 = 0x8765_4321;
    uint x2 = 0x765_4321;
    uint x3 = 0x65_4321;
    uint x4 = 0x5_4321;
    uint x5 = 0x123_1234_1234_1234;
    uint x6 = 0x123456_1234_1234;

    x1; x2; x3; x4; x5; x6;
  }
}
// ----
