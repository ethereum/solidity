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
// gas irOptimized: 80174
// gas irOptimized code: 24200
// gas legacy: 85100
// gas legacy code: 58200
// gas legacyOptimized: 80133
// gas legacyOptimized code: 22800
// getFlag() -> true
// getName() -> "abc"
