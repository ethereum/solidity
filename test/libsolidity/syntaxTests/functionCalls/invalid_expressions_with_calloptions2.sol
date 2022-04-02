contract D {}
contract C {
    function foo(int a) pure external {
      this.foo{random:5+5};
    }
}
// ----
// TypeError 9318: (73-93='this.foo{random:5+5}'): Unknown call option "random". Valid options are "salt", "value" and "gas".
