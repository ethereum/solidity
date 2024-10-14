{
    function f() {
        f()
    }
    f()
}
// Setting so that trace limit hit first
// ====
// maxRecursionDepth: 1000
// maxTraceSize: 10
// ----
// Execution result: TraceLimitReached
// Outer most variable values:
//
// Call trace:
//   [CALL] f()
//   │ [CALL] f()
//   │ │ [CALL] f()
//   │ │ │ [CALL] f()
//   │ │ │ │ [CALL] f()
//   │ │ │ │ │ [CALL] f()
//   │ │ │ │ │ │ [CALL] f()
//   │ │ │ │ │ │ │ [CALL] f()
//   │ │ │ │ │ │ │ │ [CALL] f()
//   │ │ │ │ │ │ │ │ │ [CALL] f()
//   │ │ │ │ │ │ │ │ │ │ [CALL] f()
