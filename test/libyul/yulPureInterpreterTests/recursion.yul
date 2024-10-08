{
    function fib(i) -> y
    {
        y := 1
        if gt(i, 2)
        {
            y := add(fib(sub(i, 1)), fib(sub(i, 2)))
        }
    }
    let res := fib(8)
}
// ----
// Execution result: ExecutionOk
// Outer most variable values:
//   res = 21
//
// Call trace:
//   [CALL] fib(8)
//   │ [CALL] fib(6)
//   │ │ [CALL] fib(4)
//   │ │ │ [CALL] fib(2)
//   │ │ │ │ [RETURN] 1
//   │ │ │ [CALL] fib(3)
//   │ │ │ │ [CALL] fib(1)
//   │ │ │ │ │ [RETURN] 1
//   │ │ │ │ [CALL] fib(2)
//   │ │ │ │ │ [RETURN] 1
//   │ │ │ │ [RETURN] 2
//   │ │ │ [RETURN] 3
//   │ │ [CALL] fib(5)
//   │ │ │ [CALL] fib(3)
//   │ │ │ │ [CALL] fib(1)
//   │ │ │ │ │ [RETURN] 1
//   │ │ │ │ [CALL] fib(2)
//   │ │ │ │ │ [RETURN] 1
//   │ │ │ │ [RETURN] 2
//   │ │ │ [CALL] fib(4)
//   │ │ │ │ [CALL] fib(2)
//   │ │ │ │ │ [RETURN] 1
//   │ │ │ │ [CALL] fib(3)
//   │ │ │ │ │ [CALL] fib(1)
//   │ │ │ │ │ │ [RETURN] 1
//   │ │ │ │ │ [CALL] fib(2)
//   │ │ │ │ │ │ [RETURN] 1
//   │ │ │ │ │ [RETURN] 2
//   │ │ │ │ [RETURN] 3
//   │ │ │ [RETURN] 5
//   │ │ [RETURN] 8
//   │ [CALL] fib(7)
//   │ │ [CALL] fib(5)
//   │ │ │ [CALL] fib(3)
//   │ │ │ │ [CALL] fib(1)
//   │ │ │ │ │ [RETURN] 1
//   │ │ │ │ [CALL] fib(2)
//   │ │ │ │ │ [RETURN] 1
//   │ │ │ │ [RETURN] 2
//   │ │ │ [CALL] fib(4)
//   │ │ │ │ [CALL] fib(2)
//   │ │ │ │ │ [RETURN] 1
//   │ │ │ │ [CALL] fib(3)
//   │ │ │ │ │ [CALL] fib(1)
//   │ │ │ │ │ │ [RETURN] 1
//   │ │ │ │ │ [CALL] fib(2)
//   │ │ │ │ │ │ [RETURN] 1
//   │ │ │ │ │ [RETURN] 2
//   │ │ │ │ [RETURN] 3
//   │ │ │ [RETURN] 5
//   │ │ [CALL] fib(6)
//   │ │ │ [CALL] fib(4)
//   │ │ │ │ [CALL] fib(2)
//   │ │ │ │ │ [RETURN] 1
//   │ │ │ │ [CALL] fib(3)
//   │ │ │ │ │ [CALL] fib(1)
//   │ │ │ │ │ │ [RETURN] 1
//   │ │ │ │ │ [CALL] fib(2)
//   │ │ │ │ │ │ [RETURN] 1
//   │ │ │ │ │ [RETURN] 2
//   │ │ │ │ [RETURN] 3
//   │ │ │ [CALL] fib(5)
//   │ │ │ │ [CALL] fib(3)
//   │ │ │ │ │ [CALL] fib(1)
//   │ │ │ │ │ │ [RETURN] 1
//   │ │ │ │ │ [CALL] fib(2)
//   │ │ │ │ │ │ [RETURN] 1
//   │ │ │ │ │ [RETURN] 2
//   │ │ │ │ [CALL] fib(4)
//   │ │ │ │ │ [CALL] fib(2)
//   │ │ │ │ │ │ [RETURN] 1
//   │ │ │ │ │ [CALL] fib(3)
//   │ │ │ │ │ │ [CALL] fib(1)
//   │ │ │ │ │ │ │ [RETURN] 1
//   │ │ │ │ │ │ [CALL] fib(2)
//   │ │ │ │ │ │ │ [RETURN] 1
//   │ │ │ │ │ │ [RETURN] 2
//   │ │ │ │ │ [RETURN] 3
//   │ │ │ │ [RETURN] 5
//   │ │ │ [RETURN] 8
//   │ │ [RETURN] 13
//   │ [RETURN] 21
