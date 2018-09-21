contract C {
  bytes[0] a;
  function f() public pure returns(bytes32[0][500] memory) {}
}
// ----
// TypeError: (62-84): Fixed-size multidimensional arrays are not allowed to have zero length.
