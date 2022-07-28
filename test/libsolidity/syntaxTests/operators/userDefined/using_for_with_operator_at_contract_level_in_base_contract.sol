type Int is int128;

function add(Int, Int) pure returns (Int) {}
function another_add(Int, Int) pure returns (Int) {}

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
// TypeError 3320: (144-147): Operators can only be defined in a global 'using for' directive.
// TypeError 3320: (289-300): Operators can only be defined in a global 'using for' directive.
// TypeError 2271: (491-516): Built-in binary operator + cannot be applied to types Int and Int. No matching user-defined operator found.
