{
    function fake_sstore(pos, val) {}

    f(1)
    function f(i)
    {
        if i { g(1) }

        function g(j)
        {
            if j { h() }

            f(0)

            function h()
            { g(0) }
        }
        fake_sstore(i, add(i, 7))
    }
}
// ----
// Execution result: ExecutionOk
// Outer most variable values:
//
// Call trace:
//   [CALL] f(1)
//   │ [CALL] g(1)
//   │ │ [CALL] h()
//   │ │ │ [CALL] g(0)
//   │ │ │ │ [CALL] f(0)
//   │ │ │ │ │ [CALL] fake_sstore(0, 7)
//   │ │ │ │ │ │ [RETURN]
//   │ │ │ │ │ [RETURN]
//   │ │ │ │ [RETURN]
//   │ │ │ [RETURN]
//   │ │ [CALL] f(0)
//   │ │ │ [CALL] fake_sstore(0, 7)
//   │ │ │ │ [RETURN]
//   │ │ │ [RETURN]
//   │ │ [RETURN]
//   │ [CALL] fake_sstore(1, 8)
//   │ │ [RETURN]
//   │ [RETURN]
