contract C {
  function a() public pure returns(int[0][500] memory) {}
  function b() public pure returns(uint[0][500] memory) {}
  function c() public pure returns(bytes1[0][500] memory) {}
  function d() public pure returns(bytes32[0][500] memory) {}
  function e() public pure returns(bytes[0][500] memory) {}
  function e() public pure returns(string[0][500] memory) {}
}
// ----
// TypeError 1406: (52-53): Array with zero length specified.
// TypeError 1406: (111-112): Array with zero length specified.
// TypeError 1406: (172-173): Array with zero length specified.
// TypeError 1406: (234-235): Array with zero length specified.
// TypeError 1406: (294-295): Array with zero length specified.
// TypeError 1406: (355-356): Array with zero length specified.
