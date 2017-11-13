var express = require('express');
var app = express();
var http = require('http').Server(app);
var io = require('socket.io')(http);

app.set('port', (process.env.PORT || 8000));

app.get('/',function(req,res){
  res.sendFile(__dirname + '/index.html');
});

app.use('/lib', express.static('lib'));

http.listen(3000, function(){
  console.log('listening on *:3000');
});

var SerialPort = require('serialport');
var port = new SerialPort('/dev/tty.usbmodem1421', { autoOpen: false });

port.open(function (err) {
  if (err) {
    return console.log('Error opening port: ', err.message);
  }
});

port.on('open', function() {
  // open logic
});

var isRecording = false;
var isPlaying = false;

// Socket stuff
io.on('connection', function(socket){
});

port.on('data', function (data) {
  if (data.toString()[0] === 'r' && !isRecording) {
    isRecording = true;
    io.emit('record', 'record');
  }

  if (data.toString()[0] === 's' && isRecording) {
    isRecording = false;
    io.emit('stop', 'stop');
  }

  if (data.toString()[0] === 'p') {
    // play audio
    io.emit('play', { for: 'everyone' });
  }
});
