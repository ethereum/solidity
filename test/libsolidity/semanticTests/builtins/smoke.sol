contract ClientReceipt {
}
// ====
// compileViaYul: also
// ----
// logs.numLogs() -> logs.numLogs()
// logs.logAddress(uint256): 0 -> logs.logAddress(uint256): 0
// logs.logData(uint256): 0 -> logs.logData(uint256): 0
// logs.numLogTopics(uint256): 0 -> logs.numLogTopics(uint256): 0
// logs.logTopic(uint256,uint256): 0, 0 -> logs.logTopic(uint256,uint256): 0, 0
