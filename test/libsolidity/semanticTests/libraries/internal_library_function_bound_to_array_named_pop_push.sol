library L {
    function pop(uint[2] memory a) internal {}
    function push(uint[2] memory a) internal {}
}

contract C {
    using L for uint[2];

    function test() public {
        uint[2] memory input;

        input.push();
        input.pop();
    }
}
// ----
// test() ->
