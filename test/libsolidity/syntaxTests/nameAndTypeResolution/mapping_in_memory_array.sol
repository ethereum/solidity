contract C {
    function f(uint size) public {
        var x = new mapping(uint => uint)[](4);
    }
}
// ----
// Warning: (56-61): Use of the "var" keyword is deprecated.
// TypeError: (68-91): Type cannot live outside storage.
