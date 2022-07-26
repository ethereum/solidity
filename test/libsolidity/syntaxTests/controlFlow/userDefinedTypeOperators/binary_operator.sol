struct S { bool f; }

using {add as +} for S;

function add(S storage _s, S storage) pure returns (S storage) {
    return _s;
    _s.f = true;
}

contract C {
    function get() private returns (S storage) {
	S storage s;
        return s;
    }

    function f() public {
        S storage s;
        get() + s;
    }
}

// ----
// Warning 5740: (131-142): Unreachable code.
// TypeError 3464: (238-239): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
// TypeError 3464: (311-312): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
