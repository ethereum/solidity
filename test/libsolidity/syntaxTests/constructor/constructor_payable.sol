contract C {
    constructor() payable { }
}

contract D {
    function createC() public returns (C) {
       C c = (new C){value: 1}();
       return c;
    }
}
