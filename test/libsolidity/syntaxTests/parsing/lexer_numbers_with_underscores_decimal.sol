contract C {
  function f() public pure {
    uint d1 = 654_321;
    uint d2 =  54_321;
    uint d3 =   4_321;
    uint d4 = 5_43_21;
    uint d5 = 1_2e10;
    uint d6 = 12e1_0;

    d1; d2; d3; d4; d5; d6;
  }
}
// ----
