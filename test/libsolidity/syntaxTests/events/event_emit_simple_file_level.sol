event E();

contract C {
    function f() public {
        emit E();
    }
}
