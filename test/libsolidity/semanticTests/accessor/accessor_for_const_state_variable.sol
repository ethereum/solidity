contract Lotto {
    uint256 public constant ticketPrice = 555;
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// ticketPrice() -> 555
