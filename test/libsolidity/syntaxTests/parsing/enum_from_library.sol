library L {
    enum Direction { Left, Right }
}

contract D {
    function f() public pure returns (L.Direction) {
      return L.Direction.Left;
    }
}
