using {add as +, unsub as -} for U global;

type U is uint;

function add(U, U) pure returns (U) {}
function unsub(U) pure returns (U) {}

contract C {
    function fromBool() public {
        U u;

        u + true;
        true + u;
        -true;
    }

    function fromUint() public {
        U u;
        uint32 u32;

        u + u32;
        u32 + u;
        -u32;
    }
}
// ----
// TypeError 5653: (207-215): The type of the second operand of this user-defined binary operator + does not match the type of the first operand, which is U.
// TypeError 2271: (225-233): Built-in binary operator + cannot be applied to types bool and U.
// TypeError 4907: (243-248): Built-in unary operator - cannot be applied to type bool.
// TypeError 5653: (332-339): The type of the second operand of this user-defined binary operator + does not match the type of the first operand, which is U.
// TypeError 2271: (349-356): Built-in binary operator + cannot be applied to types uint32 and U.
// TypeError 4907: (366-370): Built-in unary operator - cannot be applied to type uint32. Unary negation is only allowed for signed integers.
