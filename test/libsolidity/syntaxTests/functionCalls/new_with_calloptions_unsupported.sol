contract D { constructor() payable {} }
contract C {
    function foo() pure internal {
		new D{salt:"abc", value:3};
		new D{salt:"abc"};
		new D{value:5+5};
		new D{salt:"aabbcc"};
    }
}
// ====
// EVMVersion: <constantinople
// ----
// TypeError 5189: (90-116): Unsupported call option "salt" (requires Constantinople-compatible VMs).
// TypeError 5189: (120-137): Unsupported call option "salt" (requires Constantinople-compatible VMs).
// TypeError 5189: (161-181): Unsupported call option "salt" (requires Constantinople-compatible VMs).
