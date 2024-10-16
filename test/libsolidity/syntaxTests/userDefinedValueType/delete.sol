type MyInt is int;
function f() pure {
    MyInt i = MyInt.wrap(1);
    delete i;
}