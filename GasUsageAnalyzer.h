// GasUsageAnalyzer.h
#pragma once

#include "ASTNode.h"

class GasUsageAnalyzer
{
public:
    GasUsageAnalyzer(); // Constructor

    // Function to estimate gas usage for a specific function node
    int estimateGas(solidity::ASTNode* functionNode);

    // Function to provide optimization tips based on the gas estimate
    std::string provideOptimizationTips(solidity::ASTNode* functionNode, int gasEstimate);
};
