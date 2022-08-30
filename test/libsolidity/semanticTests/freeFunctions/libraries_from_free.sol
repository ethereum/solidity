library L {
  function pub() public pure returns (uint) {
    return 7;
  }
  function inter() internal pure returns (uint) {
    return 8;
  }
}

function fu() pure returns (uint, uint) {
  return (L.pub(), L.inter());
}

contract C {
  function f() public pure returns (uint, uint) {
    return fu();
  }
}
// ----
// library: L
// f() -> 7, 8
