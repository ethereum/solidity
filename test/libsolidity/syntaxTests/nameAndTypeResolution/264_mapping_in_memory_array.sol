contract C {
    function f(uint size) public {
        mapping(uint => uint) x = new mapping(uint => uint)[](4);
    }
}
// ----
// TypeError: (86-109): Type cannot live outside storage.
