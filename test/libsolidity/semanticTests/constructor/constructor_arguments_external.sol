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
// gas irOptimized: 106267
// gas legacy: 145838
// gas legacyOptimized: 104017
// getFlag() -> true
// getName() -> "abc"
