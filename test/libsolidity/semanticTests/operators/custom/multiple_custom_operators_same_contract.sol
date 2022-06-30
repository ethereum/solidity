type MyInt is int;
using {add as +} for MyInt;

function add(MyInt, MyInt) pure returns (bool) {
    return true;
}

contract C {
    function f() public pure returns (bool t) {
        t = MyInt.wrap(2) + MyInt.wrap(7);
    }
}
// ====
// compileViaYul: also
// ----
// f() -> true
