// TODO: Isn't it suppose to be the exact same as all_operators.sol ?

type MyInt is int;
using {add as +} for MyInt;

function add(MyInt, MyInt) pure returns (MyInt) {
    return MyInt.wrap(0);
}

contract C {
    function f() public pure returns (MyInt t) {
        t = MyInt.wrap(2) + MyInt.wrap(7);
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 0
