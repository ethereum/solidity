var testMethod = require('./helpers/test.method.js');

var method = 'getHex';

var tests = [{
    args: ['myDB', 'myKey'],
    formattedArgs: ['myDB', 'myKey'],
    result: '0xf',
    formattedResult: '0xf',
    call: 'db_'+ method
}];

testMethod.runTests('db', method, tests);

