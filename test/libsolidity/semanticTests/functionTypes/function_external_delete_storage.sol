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
// ----
// x() -> 0
// y() -> 0
// increment() ->
// y() -> 1
// set() ->
// x() -> 0xc06afe3a8444fc0004668591e8306bfb9968e79ed09de08a0000000000000000
// increment() ->
// y() -> 2
// incrementIndirectly() ->
// y() -> 3
// deleteFunction() ->
// increment() ->
// y() -> 4
// incrementIndirectly() -> FAILURE
// y() -> 4
