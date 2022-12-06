struct S {
    uint8 a;
}

using {
    add as +,
    sub as -,
    mul as *
} for S;

function add(S memory, S memory) pure returns (S memory) {}
function sub(S calldata, S calldata) pure returns (S calldata) {}
function mul(S storage, S storage) pure returns (S storage) {}

contract C {
    S s;
    function test(S calldata c) public {
        S memory m;

        // operator accepts only memory
        m + c;
        m + s;
        c + m;
        s + m;

        // operator accepts only calldata
        c - m;
        c - s;
        m - c;
        s - c;

        // operator accepts only storage
        s * c;
        s * m;
        c * s;
        m * s;
    }
}
// ----
// TypeError 5653: (408-413): The type of the second operand of this user-defined binary operator + does not match the type of the first operand, which is struct S memory.
// TypeError 5653: (423-428): The type of the second operand of this user-defined binary operator + does not match the type of the first operand, which is struct S memory.
// TypeError 1349: (438-443): User-defined binary operator + cannot be applied to type struct S calldata. None of the available definitions accepts calldata arguments.
// TypeError 1349: (453-458): User-defined binary operator + cannot be applied to type struct S storage ref. None of the available definitions accepts storage arguments.
// TypeError 5653: (511-516): The type of the second operand of this user-defined binary operator - does not match the type of the first operand, which is struct S calldata.
// TypeError 5653: (526-531): The type of the second operand of this user-defined binary operator - does not match the type of the first operand, which is struct S calldata.
// TypeError 1349: (541-546): User-defined binary operator - cannot be applied to type struct S memory. None of the available definitions accepts memory arguments.
// TypeError 1349: (556-561): User-defined binary operator - cannot be applied to type struct S storage ref. None of the available definitions accepts storage arguments.
// TypeError 5653: (613-618): The type of the second operand of this user-defined binary operator * does not match the type of the first operand, which is struct S storage pointer.
// TypeError 5653: (628-633): The type of the second operand of this user-defined binary operator * does not match the type of the first operand, which is struct S storage pointer.
// TypeError 1349: (643-648): User-defined binary operator * cannot be applied to type struct S calldata. None of the available definitions accepts calldata arguments.
// TypeError 1349: (658-663): User-defined binary operator * cannot be applied to type struct S memory. None of the available definitions accepts memory arguments.
