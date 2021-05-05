pragma abicoder v2;

type Small is uint16;
type Left is bytes2;
struct S { uint8 a; Small b; Left c; uint8 d; }

contract C {
    S public s;
    Small[] public small;
    Left[] public l;
    function f(S memory _s) public {
        s = _s;
    }
    function g(Small[] memory _small) public returns (Small[] memory) {
        small = _small;
        return small;
    }
    function h(Left[] memory _left) public returns (Left[] memory) {
        l = _left;
        return l;
    }
}
// ====
// compileViaYul: also
// ----
// s() -> 0, 0, 0x00, 0
// f((uint8,uint16,bytes2,uint8)): 1, 0xff, "ab", 15 ->
// gas irOptimized: 44473
// gas legacy: 46213
// gas legacyOptimized: 44671
// s() -> 1, 0xff, 0x6162000000000000000000000000000000000000000000000000000000000000, 15
// g(uint16[]): 0x20, 3, 1, 2, 3 -> 0x20, 3, 1, 2, 3
// gas irOptimized: 69555
// gas legacy: 76557
// gas legacyOptimized: 74834
// small(uint256): 0 -> 1
// small(uint256): 1 -> 2
// h(bytes2[]): 0x20, 3, "ab", "cd", "ef" -> 0x20, 3, "ab", "cd", "ef"
// gas irOptimized: 69617
// gas legacy: 76238
// gas legacyOptimized: 74921
// l(uint256): 0 -> 0x6162000000000000000000000000000000000000000000000000000000000000
// l(uint256): 1 -> 0x6364000000000000000000000000000000000000000000000000000000000000
