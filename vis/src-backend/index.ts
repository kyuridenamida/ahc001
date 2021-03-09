import * as express from "express";

const app = require('express')();
const http = require('http').Server(app);
const io = require('socket.io')(http);

const port = Number(process.env.PORT || 8888);
app.use(express.json())
app.post('/json/', function (req: any, res: any) {
    io.sockets.emit("publish", req.body);
    res.send("ok")
});

app.get('/', function (req: any, res: any) {
    res.sendFile(__dirname + '/index.html');
});

http.listen(port, () => {
    console.info('Backend started on port: ' + port);
});
