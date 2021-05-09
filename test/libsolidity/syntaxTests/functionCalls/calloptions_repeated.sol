contract D {}
contract C {
    function foo(int a) payable external {
        this.foo{value:2, gas: 5}{gas:2};
        (this.foo{value:2, gas: 5}){gas:2};
        this.foo{value:2, gas: 5}{value:6};
        this.foo{gas:2, value: 5}{value:2, gas:5};
        new D{salt:"abc"}{salt:"a"}();
    }
}
// ====
// EVMVersion: >=constantinople
// ----
// TypeError 1645: (78-110): Function call options have already been set, you have to combine them into a single {...}-option.
// TypeError 1645: (120-154): Function call options have already been set, you have to combine them into a single {...}-option.
// TypeError 1645: (164-198): Function call options have already been set, you have to combine them into a single {...}-option.
// TypeError 1645: (208-249): Function call options have already been set, you have to combine them into a single {...}-option.
// TypeError 1645: (259-286): Function call options have already been set, you have to combine them into a single {...}-option.
