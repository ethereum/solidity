type MyInt is int;
contract C {
    function f() public returns (MyInt a, int b) {
        (MyInt).wrap;
        a = (MyInt).wrap(5);
        (MyInt).unwrap;
        b = (MyInt).unwrap((MyInt).wrap(10));
    }
}
// ----
// f() -> 5, 10
