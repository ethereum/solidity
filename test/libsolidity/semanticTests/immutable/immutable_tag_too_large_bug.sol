contract C {
  int immutable x = 1;
  int immutable y = 3;

  function f() public payable returns(int, int) {
    uint a = uint(y / 1e8);
    int256 b = x * y;
    int24 c = int24(b * b >> 128);
    int24 d = int24((100 * y + 1) >> 128);
    int24 e = int24(x >> 128);
    int256 f = x * 2;
    if (c < 0) {
      int256 g = (x * x * y) / x;
      require((y >= 0 && g <= x) || (y < 0 && (x - y) > x));
      if (b >= f) {
        require(x <= int256(uint256(type(uint168).max)) && x >= 0, "");
        b = (b * b) / f;
        for (a = 0; a < a; a++) {
          uint8 b;
          assembly  {
            b := and(mload(a), 0xFF)
          }
        }
        b += f;
      }
      require(d % e != 0);
      c = -c;
    }
    return (x, ((x * (x - y)) / (x + y)));
  }

  constructor () {
    x--;
    --x;
    y++;
    ++y;
    --y;
  }
}
// ====
// compileViaYul: true
// ----
// constructor() ->
// gas irOptimized: 73171
// gas irOptimized code: 291200
// gas legacy: 83499
// gas legacy code: 408800
// f() -> -1, 1
