contract C {
  function f() public {
    bytes[32] memory a;
    a[8**90][8**90][1 - 8**90];
  }
}
