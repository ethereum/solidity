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

        c + c; // operator accepts memory, arguments are calldata
        s + s; // operator accepts memory, arguments are storage

        m - m; // operator accepts calldata, arguments are memory
        s - s; // operator accepts calldata, arguments are storage

        c * c; // operator accepts storage, arguments are calldata
        m * m; // operator accepts storage, arguments are memory
    }
}
// ----
// TypeError 1349: (368-373): User-defined binary operator + cannot be applied to type struct S calldata. None of the available definitions accepts calldata arguments.
// TypeError 1349: (434-439): User-defined binary operator + cannot be applied to type struct S storage ref. None of the available definitions accepts storage arguments.
// TypeError 1349: (500-505): User-defined binary operator - cannot be applied to type struct S memory. None of the available definitions accepts memory arguments.
// TypeError 1349: (566-571): User-defined binary operator - cannot be applied to type struct S storage ref. None of the available definitions accepts storage arguments.
// TypeError 1349: (634-639): User-defined binary operator * cannot be applied to type struct S calldata. None of the available definitions accepts calldata arguments.
// TypeError 1349: (701-706): User-defined binary operator * cannot be applied to type struct S memory. None of the available definitions accepts memory arguments.
