contract A { event e(uint a, string b); }
contract C is A {
    function f() public {
        emit A.e(2, "abc");
        emit A.e({b: "abc", a: 8});
    }
}
