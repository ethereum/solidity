type Int is int;

using {add as +} for Int;

function add(Int, Int) pure returns (Int) {
    return Int.wrap(0);
}

function another_add(Int, Int) pure returns (Int) {
    return Int.wrap(0);
}

contract C {
    using {another_add as +} for Int;

    function f() public {
        Int.wrap(0) + Int.wrap(0);
    }
}

// ----
// TypeError 2271: (281-306): Binary operator + not compatible with types Int and Int. Multiple user-defined functions provided for this operator.
