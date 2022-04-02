contract C {
    function f(uint size) public {
        uint[] memory x = new uint[]();
    }
}
// ----
// TypeError 6160: (74-86='new uint[]()'): Wrong argument count for function call: 0 arguments given but expected 1.
