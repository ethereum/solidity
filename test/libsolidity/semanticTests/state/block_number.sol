contract C {
    constructor() {}
    function f() public returns (uint) {
        return block.number;
    }
}
// ----
// constructor()
// f() -> 2
// f() -> 3
