type Int is int128;

function add(Int, Int) pure returns (Int) {
    return Int.wrap(3);
}

function another_add(Int, Int) pure returns (Int) {
    return Int.wrap(7);
}

contract B {
    using {add as +} for Int;

    function f() pure public returns (Int) {
        return Int.wrap(0) + Int.wrap(0);
    }
}

contract C is B {
    using {another_add as +} for Int;

    function g() pure public returns (Int) {
        return Int.wrap(0) + Int.wrap(0);
    }
}

contract D is B {
    function h() pure public returns (Int) {
        return Int.wrap(0) + Int.wrap(0);
    }
}

// ----
// TypeError 2271: (542-567): Binary operator + not compatible with types Int and Int. No matching user-defined operator found.
