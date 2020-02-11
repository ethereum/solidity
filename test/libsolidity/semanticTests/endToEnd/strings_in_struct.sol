contract buggystruct {
    Buggy public bug;

    struct Buggy {
        uint first;
        uint second;
        uint third;
        string last;
    }

    constructor() public {
        bug = Buggy(10, 20, 30, "asdfghjkl");
    }

    function getFirst() public returns(uint) {
        return bug.first;
    }

    function getSecond() public returns(uint) {
        return bug.second;
    }

    function getThird() public returns(uint) {
        return bug.third;
    }

    function getLast() public returns(string memory) {
        return bug.last;
    }
}

// ----
// getFirst() -> 10
// getSecond() -> 20
// getThird() -> 30
// getLast() -> 0x20, 9, "asdfghjkl"
