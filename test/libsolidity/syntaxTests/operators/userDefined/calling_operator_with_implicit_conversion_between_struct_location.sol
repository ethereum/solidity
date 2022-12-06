using {add as +, unsub as -} for S;
using {mul as *, not as !} for S;

struct S {
    uint x;
}

function add(S memory, S memory) returns (S memory) {}
function mul(S storage, S storage) returns (S storage) {}
function unsub(S memory) returns (S memory) {}
function not(S storage) returns (S storage) {}

contract C {
    S sRef;

    function storageToMemory() public {
        S storage sPtr;
        S memory sMem;

        sMem + sPtr;
        sPtr + sMem;
        sPtr + sPtr;

        sMem + sRef;
        sRef + sMem;
        sRef + sRef;

        sRef + sPtr;
        sPtr + sRef;

        -sPtr;
        -sRef;
    }

    function memoryToStorage() public {
        S memory sMem;
        S storage sPtr;

        sMem * sPtr;
        sPtr * sMem;
        sMem * sMem;

        sMem * sRef;
        sRef * sMem;
        sMem * sMem;

        sRef * sPtr;
        sPtr * sRef;

        !sMem;
    }
}
// ----
// TypeError 5653: (427-438): The type of the second operand of this user-defined binary operator + does not match the type of the first operand, which is struct S memory.
// TypeError 1349: (448-459): User-defined binary operator + cannot be applied to type struct S storage pointer. None of the available definitions accepts storage arguments.
// TypeError 1349: (469-480): User-defined binary operator + cannot be applied to type struct S storage pointer. None of the available definitions accepts storage arguments.
// TypeError 5653: (491-502): The type of the second operand of this user-defined binary operator + does not match the type of the first operand, which is struct S memory.
// TypeError 1349: (512-523): User-defined binary operator + cannot be applied to type struct S storage ref. None of the available definitions accepts storage arguments.
// TypeError 1349: (533-544): User-defined binary operator + cannot be applied to type struct S storage ref. None of the available definitions accepts storage arguments.
// TypeError 1349: (555-566): User-defined binary operator + cannot be applied to type struct S storage ref. None of the available definitions accepts storage arguments.
// TypeError 1349: (576-587): User-defined binary operator + cannot be applied to type struct S storage pointer. None of the available definitions accepts storage arguments.
// TypeError 5652: (598-603): User-defined unary operator - cannot be applied to type struct S storage pointer. None of the available definitions accepts storage arguments.
// TypeError 5652: (613-618): User-defined unary operator - cannot be applied to type struct S storage ref. None of the available definitions accepts storage arguments.
// TypeError 1349: (723-734): User-defined binary operator * cannot be applied to type struct S memory. None of the available definitions accepts memory arguments.
// TypeError 5653: (744-755): The type of the second operand of this user-defined binary operator * does not match the type of the first operand, which is struct S storage pointer.
// TypeError 1349: (765-776): User-defined binary operator * cannot be applied to type struct S memory. None of the available definitions accepts memory arguments.
// TypeError 1349: (787-798): User-defined binary operator * cannot be applied to type struct S memory. None of the available definitions accepts memory arguments.
// TypeError 5653: (808-819): The type of the second operand of this user-defined binary operator * does not match the type of the first operand, which is struct S storage pointer.
// TypeError 1349: (829-840): User-defined binary operator * cannot be applied to type struct S memory. None of the available definitions accepts memory arguments.
// TypeError 5652: (894-899): User-defined unary operator ! cannot be applied to type struct S memory. None of the available definitions accepts memory arguments.
