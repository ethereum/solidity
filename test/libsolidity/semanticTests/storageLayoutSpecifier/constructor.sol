contract A {
    uint public x;
}

contract B is A {
    int8 public y;
}

contract C is B layout at 7 {
    uint32 public z;

    constructor(uint _x, int8 _y, uint32 _z) {
        x = _x + 1;
        y = _y + 2;
        z = _z + 3;
    }

}
// ----
// constructor(): 1, 2, 3
// gas irOptimized: 104162
// gas irOptimized code: 29800
// gas legacy: 114749
// gas legacy code: 71400
// gas legacyOptimized: 106296
// gas legacyOptimized code: 31400
// gas ssaCFGOptimized: 104069
// gas ssaCFGOptimized code: 28600
// x() -> 2
// y() -> 4
// z() -> 6
