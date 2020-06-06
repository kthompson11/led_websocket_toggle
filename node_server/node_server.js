var net = require('net');
var fs = require('fs');

var client = net.createConnection("/tmp/led_ctrl");

function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

message = {
  periodMS: 1000
};

client.on('connect',async function() {
  console.log('unix server socket connected on /tmp/unixSocket');
  
  while (true) {
    await sleep(10000);
    message.periodMS = 1000;
    client.write(JSON.stringify(message));
    await sleep(10000);
    message.periodMS = 333;
    client.write(JSON.stringify(message));
    await sleep(10000);
    message.periodMS = 111;
    client.write(JSON.stringify(message));
    await sleep(10000);
    message.periodMS = 37;
    client.write(JSON.stringify(message));
    await sleep(10000);
    message.periodMS = 12;
    client.write(JSON.stringify(message));
  }
  
  client.end();
});

client.on('data', function(data) {
  console.log("" + data);
});
