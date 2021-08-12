// Invoke some features that use memory and test that they do not interfere with each other.
contract Helper {
    uint256 public flag;

    constructor(uint256 x) {
        flag = x;
    }
}


contract Main {
    mapping(uint256 => uint256) map;

    function f(uint256 x) public returns (uint256) {
        map[x] = x;
        return
            (new Helper(uint256(keccak256(abi.encodePacked(this.g(map[x]))))))
                .flag();
    }

    function g(uint256 a) public returns (uint256) {
        return map[a];
    }
}
// ====
// compileViaYul: also
// ----
// f(uint256): 0x34 -> 0x46bddb1178e94d7f2892ff5f366840eb658911794f2c3a44c450aa2c505186c1
// gas irOptimized: 113776
// gas legacy: 126852
// gas legacyOptimized: 114015
