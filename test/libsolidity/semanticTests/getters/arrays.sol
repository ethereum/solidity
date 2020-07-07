contract C {
    uint8[][2] public a;
    constructor() {
        a[1].push(3);
        a[1].push(4);
    }
}
// ====
// compileViaYul: also
// ----
// a(uint256,uint256): 0, 0 -> FAILURE
// a(uint256,uint256): 1, 0 -> 3
// a(uint256,uint256): 1, 1 -> 4
// a(uint256,uint256): 2, 0 -> FAILURE
