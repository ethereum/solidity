var BigNumber = require('bignumber.js');
var web3 = require('../index');
var testMethod = require('./helpers/test.method.js');

var method = 'getBalance';

var tests = [{
    args: [301, 2],
    formattedArgs: ['0x000000000000000000000000000000000000012d', '0x2'],
    result: '0x31981',
    formattedResult: new BigNumber('0x31981', 16),
    call: 'eth_'+ method
},{
    args: ['0x12d', '0x1'],
    formattedArgs: ['0x000000000000000000000000000000000000012d', '0x1'],
    result: '0x31981',
    formattedResult: new BigNumber('0x31981', 16),
    call: 'eth_'+ method
}, {
    args: [0x12d, 0x1],
    formattedArgs: ['0x000000000000000000000000000000000000012d', '0x1'],
    result: '0x31981',
    formattedResult: new BigNumber('0x31981', 16),
    call: 'eth_'+ method
}, {
    args: [0x12d],
    formattedArgs: ['0x000000000000000000000000000000000000012d', web3.eth.defaultBlock],
    result: '0x31981',
    formattedResult: new BigNumber('0x31981', 16),
    call: 'eth_'+ method
}, {
    args: ['0xdbdbdb2cbd23b783741e8d7fcf51e459b497e4a6', 0x1],
    formattedArgs: ['0xdbdbdb2cbd23b783741e8d7fcf51e459b497e4a6', '0x1'],
    result: '0x31981',
    formattedResult: new BigNumber('0x31981', 16),
    call: 'eth_'+ method
}, {
    args: ['dbdbdb2cbd23b783741e8d7fcf51e459b497e4a6', 0x1],
    formattedArgs: ['0xdbdbdb2cbd23b783741e8d7fcf51e459b497e4a6', '0x1'],
    result: '0x31981',
    formattedResult: new BigNumber('0x31981', 16),
    call: 'eth_'+ method
}, {
    args: ['1255171934823351805979544608257289442498956485798', 0x1],
    formattedArgs: ['0xdbdbdb2cbd23b783741e8d7fcf51e459b497e4a6', '0x1'],
    result: '0x31981',
    formattedResult: new BigNumber('0x31981', 16),
    call: 'eth_'+ method
}, {
    args: ['1255171934823351805979544608257289442498956485798'],
    formattedArgs: ['0xdbdbdb2cbd23b783741e8d7fcf51e459b497e4a6', 'latest'],
    result: '0x31981',
    formattedResult: new BigNumber('0x31981', 16),
    call: 'eth_'+ method
}, {
    args: ['0x00000000000000000000aaaaaaaaaaaaaaaaaaa'],
    formattedArgs: ['0x000000000000000000000aaaaaaaaaaaaaaaaaaa', 'latest'],
    result: '0x31981',
    formattedResult: new BigNumber('0x31981', 16),
    call: 'eth_'+ method
}];

testMethod.runTests('eth', method, tests);

