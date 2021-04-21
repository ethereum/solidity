contract C {
    mapping(uint=>uint)[] array;
    mapping(uint=>uint) map;
    function f() public {
        array.pop();
    }
}
// ----
