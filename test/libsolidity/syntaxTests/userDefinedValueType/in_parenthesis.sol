type MyInt is int;
function f() pure {
    (MyInt).wrap;
    (MyInt).wrap(5);
    (MyInt).unwrap;
    (MyInt).unwrap(MyInt.wrap(5));
}
