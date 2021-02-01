contract C {
    constructor() {}
    function f() public returns (uint) {
        return block.timestamp;
    }
}
// ====
// compileViaYul: also
// ----
// constructor() # This is the 1st block #
// gas ir: 130626
// gas legacy: 100305
// f() -> 0x1e # This is the 2nd block (each block is "15 seconds") #
// f() -> 0x2d # This is the 3rd block #
