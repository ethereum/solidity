contract test {
    uint[] a;
    uint[]  b;
    function f() public {
      uint[] storage s1 = a;
      uint[] memory s2 = new uint[](42);
      uint[] storage s3 = b;
      s1.push(42);
      s2[3] = 12;
      s3.push(42);
    }
}
// ----
