struct S {
    int value;
}

using {add as +, unsub as -} for S;

function add(S storage a, S storage b) returns (S storage) {
    a.value = a.value + b.value;
    return a;
}

function unsub(S storage a) returns (S storage) {
    a.value = -a.value;
    return a;
}

contract C {
    S a = S(10);
    S b = S(3);

    function addStorageRef() public returns (int) {
        return (a + b).value;
    }

    function addStoragePtr() public returns (int) {
        // Storage pointers are technically a different type internally. The point of this test is
        // to make sure they go find through both the codegen and the analysis.
        S storage c = a;
        S storage d = b;

        return (c + d).value;
    }

    function addStoragePtrAndRef() public returns (int) {
        S storage c = a;

        return (a + c).value;
    }

    function unsubStorageRef() public returns (int) {
        return (-a).value;
    }

    function unsubStoragePtr() public returns (int) {
        S storage c = a;

        return (-c).value;
    }
}
// ----
// addStorageRef() -> 13
// addStoragePtr() -> 16
// addStoragePtrAndRef() -> 32
// unsubStorageRef() -> -32
// unsubStoragePtr() -> 32
