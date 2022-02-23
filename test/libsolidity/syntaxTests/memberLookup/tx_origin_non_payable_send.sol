contract C {
    function f() public {
        (tx.origin).send(10);
    }
}
// ----
// TypeError 9862: (47-63): "send" and "transfer" are only available for objects of type "address payable", not "address".
