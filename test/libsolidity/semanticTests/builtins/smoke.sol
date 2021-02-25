contract ClientReceipt {
}
// ====
// compileViaYul: also
// ----
// smoke.test0 ->
// smoke.test0 -> smoke.test0
// smoke.test1: 1 -> 1
// smoke.test1: 1 -> smoke.test1: 1
// smoke.test2: 2, 3 -> 2, 3
// smoke.test2: 2, 3 -> smoke.test2: 2, 3
// smoke.test2: 2, 3, 4 -> FAILURE
// smoke.test2: 2, 3, 4 -> smoke.test2: 2, 3, 4
// smoke.test0 ->
// smoke_test3: 2, 3 -> 2, 3
// smoke_test3: 2, 3 -> smoke_test3: 2, 3
// smoke_test3: 2, 3, 4 -> FAILURE
// smoke.reaction ->
// - 1 / 1.
// smoke.reaction ->
// - 1 / 2.
// - 2 / 2.
// smoke.reaction ->
// - 1 / 3.
// - 2 / 3.
// - 3 / 3.
// smoke.reaction ->
// - 1 / 4.
// - 2 / 4.
// - 3 / 4.
// - 4 / 4.
// smoke.reaction ->
// - 1 / 5.
// - 2 / 5.
// - 3 / 5.
// - 4 / 5.
// - 5 / 5.
