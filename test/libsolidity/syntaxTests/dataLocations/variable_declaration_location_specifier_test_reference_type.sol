contract test {
    uint[] a;
    uint[]  b;
    function f() public {
      uint[] storage s1 = a;
      uint[] memory s2 = new uint[](42);
      uint[] s3 = b;
      s1.push(42);
      s2[3] = 12;
      s3.push(42);
    }
}
// ----
// Warning: (147-156): Variable is declared as a storage pointer. Use an explicit "storage" keyword to silence this warning.
