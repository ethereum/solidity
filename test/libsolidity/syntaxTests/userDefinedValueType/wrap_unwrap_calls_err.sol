type MyInt is int;
function f() {
    MyInt.wrap(5, 6, 7);
    MyInt.wrap({test: 5});
    MyInt.wrap();
    MyInt.unwrap(5);
    MyInt.unwrap({test: 5});
    MyInt.unwrap(MyInt.wrap(1), MyInt.wrap(2));
}
// ----
// TypeError 6160: (38-57): Wrong argument count for function call: 3 arguments given but expected 1.
// TypeError 4974: (63-84): Named argument "test" does not match function declaration.
// TypeError 6160: (90-102): Wrong argument count for function call: 0 arguments given but expected 1.
// TypeError 9553: (121-122): Invalid type for argument in function call. Invalid implicit conversion from int_const 5 to MyInt requested.
// TypeError 4974: (129-152): Named argument "test" does not match function declaration.
// TypeError 6160: (158-200): Wrong argument count for function call: 2 arguments given but expected 1.
