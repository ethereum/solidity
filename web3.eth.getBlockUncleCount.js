var chai = require('chai');
var web3 = require('../index');
var method = 'getBlockUncleCount';
var testMethod = require('./helpers/test.method.js');


var tests = [{
    args: ['0x47d33b27bb249a2dbab4c0612bf9caf4c1950855'],
    formattedArgs: ['0x47d33b27bb249a2dbab4c0612bf9caf4c1950855'],
    result: '0xb',
    formattedResult: 11,
    call: 'eth_getUncleCountByBlockHash'
},{
    args: [436],
    formattedArgs: ['0x1b4'],
    result: '0xb',
    formattedResult: 11,
    call: 'eth_getUncleCountByBlockNumber'
},{
    args: ['pending'],
    formattedArgs: ['pending'],
    result: '0xb',
    formattedResult: 11,
    call: 'eth_getUncleCountByBlockNumber'
}];

testMethod.runTests('eth', method, tests);
