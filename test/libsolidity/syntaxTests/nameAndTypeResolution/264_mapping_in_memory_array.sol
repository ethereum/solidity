contract C {
    function f(uint size) public {
        mapping(uint => uint) storage x = new mapping(uint => uint)[](4);
    }
}
// ----
// TypeError 1164: (94-117): Array containing a (nested) mapping cannot be constructed in memory.
