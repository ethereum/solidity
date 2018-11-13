contract C {
    uint[] data;
    function f(uint[] memory x) public {
        data = x;
    }
}
