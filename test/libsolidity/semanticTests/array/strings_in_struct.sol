contract buggystruct {
    Buggy public bug;

    struct Buggy {
        uint256 first;
        uint256 second;
        uint256 third;
        string last;
    }

    constructor() {
        bug = Buggy(10, 20, 30, "asdfghjkl");
    }

    function getFirst() public returns (uint256) {
        return bug.first;
    }

    function getSecond() public returns (uint256) {
        return bug.second;
    }

    function getThird() public returns (uint256) {
        return bug.third;
    }

    function getLast() public returns (string memory) {
        return bug.last;
    }
}
// ====
// compileToEwasm: also
// ----
// getFirst() -> 0x0a
// getSecond() -> 0x14
// getThird() -> 0x1e
// getLast() -> 0x20, 0x09, "asdfghjkl"
