contract C {
    function f() public {
        (tx.origin).transfer(10);
    }
}
// ----
// TypeError 9862: (47-67): "send" and "transfer" are only available for objects of type "address payable", not "address".
