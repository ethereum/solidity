contract C {
    function() external public x;
    uint public y = 0;

    function increment() public {
        ++y;
    }

    function set() external {
        x = this.increment;
    }

    function incrementIndirectly() public {
        x();
    }

    function deleteFunction() public {
        // used to lead to an ICE during IR
        delete x;
    }
}
// ====
// compileViaYul: also
// ----
// x() -> 0
// y() -> 0
// increment() ->
// y() -> 1
// set() ->
// x() -> 0xfdd67305928fcac8d213d1e47bfa6165cd0b87bd09de08a0000000000000000
// increment() ->
// y() -> 2
// incrementIndirectly() ->
// y() -> 3
// deleteFunction() ->
// increment() ->
// y() -> 4
// incrementIndirectly() -> FAILURE
// y() -> 4
