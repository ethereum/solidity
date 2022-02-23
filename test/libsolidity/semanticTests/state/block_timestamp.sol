contract C {
    constructor() {}
    function f() public returns (uint) {
        return block.timestamp;
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// constructor() # This is the 1st block #
// f() -> 0x1e # This is the 2nd block (each block is "15 seconds") #
// f() -> 0x2d # This is the 3rd block #
