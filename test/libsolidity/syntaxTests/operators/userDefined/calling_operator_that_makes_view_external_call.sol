type Int is int16;
using {add as +, unsub as -} for Int global;

IAdder constant ADDER = IAdder(address(0));

function add(Int x, Int y) pure returns (Int) {
    return ADDER.mul(x, y);
}

function unsub(Int x) pure returns (Int) {
    return ADDER.inc(x);
}

interface IAdder {
    function mul(Int, Int) external view returns (Int);
    function inc(Int) external view returns (Int);
}
// ----
// TypeError 2527: (169-184): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (243-255): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
