contract Main {
    bytes3 name;
    bool flag;

    constructor(bytes3 x, bool f) {
        name = x;
        flag = f;
    }

    function getName() public returns (bytes3 ret) {
        return name;
    }

    function getFlag() public returns (bool ret) {
        return flag;
    }
}
// ====
// compileViaYul: also
// ----
// constructor(): "abc", true
// gas ir: 242196
// gas irOptimized: 148076
// gas legacy: 165240
// gas legacyOptimized: 130015
// getFlag() -> true
// getName() -> "abc"
