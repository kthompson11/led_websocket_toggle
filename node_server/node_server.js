var net = require('net');
var fs = require('fs');

const IPC_SOCKET_ADDRESS = '/tmp/led_ctrl';
const WEBSOCKET_PORT = 8010;
const MAX_OPEN_WEBSOCKETS = 3;

var shutdownServer = false;
var client;
var isClientConnected = false;

// TODO: determine if lock needed for openWebsockets
var openWebsockets = {}; // (websocketID: string => websocket)
var nextWebsocketID = 0;


function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

/*************************** IPC Client Socket ***************************/
function ipcConnect() {
  client = new net.Socket();
  client.connect(IPC_SOCKET_ADDRESS, function () {
    console.log('IPC connection established on %s', IPC_SOCKET_ADDRESS);
    isClientConnected = true;
  });

  client.on('data', function (data) {
    var message = JSON.parse(data);
    if (message.websocketID === undefined) {
      // broadcast to all connected websockets
      for (var id in openWebsockets) {
        var destSocket = openWebsockets[id];
        destSocket.send(data);
      }
    } else {
      // get destination socket
      var id = message.websocketID.toString();
      var destSocket = openWebsockets[id];

      // strip id from message
      delete message.webSocketID;

      // send message if desination is valid
      if (destSocket !== undefined) {
        destSocket.send(JSON.stringify(message));
      }
    }
  });

  client.on('close', async function (had_error) {
    isClientConnected = false;
    if (!shutdownServer) {
      // attempt to reconnect
      console.log('Attempting to reconnect to %s', IPC_SOCKET_ADDRESS);
      await sleep(5000);
      ipcConnect();
    }
  });

  client.on('error', function (err) {});
}

ipcConnect();


/*************************** Websockets ***************************/
function sanitize(jsonString) {
  // TODO: implement sanitize
  return jsonString;
}

const WebSocket = require('ws');
const wss = new WebSocket.Server({ port: WEBSOCKET_PORT });

wss.on('connection', function connection(ws) {
  // assign websocket ID and add to list of open websockets
  var websocketID = nextWebsocketID.toString();
  nextWebsocketID = nextWebsocketID + 1;
  openWebsockets[websocketID] = ws;
  console.log('Websocket connection established (id = %s)', websocketID);

  ws.on('message', function (data) {
    // TODO: sanitize input and/or handle errors
    var request = JSON.parse(data);
    var type = request.type;
    if (type === 'getState') {
      // append websocket id and pass along request
      request.websocketID = ws.websocketID;
      client.write(JSON.stringify(request));
    } else if (type === 'modify') {
      // pass request along
      client.write(data);
    } else { 
      // bad request
    }
  });

  ws.on('close', function (code, reason) {
    console.log('Websocket connection closed (id = %s)', websocketID);
    // remove from list of open websockets
    delete openWebsockets[websocketID];
    // stop the timer
    clearInterval(ws.intervalTimer);
  });

  // attempt ping-pong every 10 seconds
  ws.intervalTimer = setInterval(function () {
    ws.ping();
  }, 10000);
});


/*************************** Signal Handler ***************************/
process.on('SIGINT', function () {
  console.log('Shutting down server');
  shutdownServer = true;
  wss.close();
  client.destroy();
  //process.exit(0);
});