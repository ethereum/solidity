contract C {
    function f(uint size) public {
        mapping(uint => uint) storage x = new mapping(uint => uint)[](4);
    }
}
// ----
// TypeError: (94-117): Type cannot live outside storage.
