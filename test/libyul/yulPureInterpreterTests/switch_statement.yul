{
    let x
    switch 7
    case 7 { x := 1 }
    case 3 { x := 2 }
    default { x := 3 }
}
// ----
// Execution result: ExecutionOk
// Outer most variable values:
//   x = 1
//
// Call trace:
