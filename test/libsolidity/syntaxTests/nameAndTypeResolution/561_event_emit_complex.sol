contract C {
    event e(uint a, string b);
    function f() public {
        emit e(2, "abc");
        emit e({b: "abc", a: 8});
    }
}
