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
// ----
// constructor(): "abc", true
// gas irOptimized: 104374
// gas legacy: 143300
// gas legacyOptimized: 102933
// getFlag() -> true
// getName() -> "abc"
