type Int is int;

using {add as +} for Int global;

function add(Int, Int) pure returns (Int) {}
function another_add(Int, Int) pure returns (Int) {}

contract B {
    using {another_add as +} for Int;

    function f() public {
        Int.wrap(0) + Int.wrap(0);
    }
}

contract C is B {
    function g() public {
        Int.wrap(0) + Int.wrap(0);
    }
}
// ----
// TypeError 3320: (175-186): Operators can only be defined in a global 'using for' directive.
// TypeError 4705: (175-186): User-defined binary operator + has more than one definition matching the operand type visible in the current scope.
