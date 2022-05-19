contract C {
  uint[] data;

  function val() public returns (bool) {
    assembly {
        sstore(0, 2)
        mstore(0, 0)
        sstore(keccak256(0, 32), 234)
        // A bug in the caching mechanism previously caused keccak256(0, 23) to be the same as
        // keccak256(0, 32), leading to `data[1] == 123` being true.
        sstore(add(keccak256(0, 23), 1), 123)
    }
    assert(data[1] != 123);
    assert(data[1] == 0);
    return true;
  }
}
// ----
// val() -> true
