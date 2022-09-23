struct S {
    uint8 a;
}

using {add as +,
       sub as -,
       mul as *} for S;

function add(S memory a, S memory) pure returns (S memory) {
    return a;
}

function sub(S calldata a, S calldata) pure returns (S calldata) {
    return a;
}

function mul(S storage a, S storage) pure returns (S storage) {
    return a;
}

contract C {
    S s;
    function test(S calldata c) public {
        S memory m;

        c + c; // calldata to memory
        s + s; // storage to memory

        m - m; // memory to calldata
        s - s; // storage to calldata

        c * c; // calldata to storage
        m * m; // memory to storage
    }
}
// ----
// TypeError 2271: (421-426): Built-in binary operator + cannot be applied to types struct S calldata and struct S calldata. No matching user-defined operator found.
// TypeError 2271: (458-463): Built-in binary operator + cannot be applied to types struct S storage ref and struct S storage ref. No matching user-defined operator found.
// TypeError 2271: (495-500): Built-in binary operator - cannot be applied to types struct S memory and struct S memory. No matching user-defined operator found.
// TypeError 2271: (532-537): Built-in binary operator - cannot be applied to types struct S storage ref and struct S storage ref. No matching user-defined operator found.
// TypeError 2271: (571-576): Built-in binary operator * cannot be applied to types struct S calldata and struct S calldata. No matching user-defined operator found.
// TypeError 2271: (609-614): Built-in binary operator * cannot be applied to types struct S memory and struct S memory. No matching user-defined operator found.
