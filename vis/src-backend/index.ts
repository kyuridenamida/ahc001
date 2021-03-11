import * as express from "express";
import ErrnoException = NodeJS.ErrnoException;

const app = require('express')();
const http = require('http').Server(app);
const io = require('socket.io')(http);
const fs = require('fs');

const port = Number(process.env.PORT || 8888);
app.use(express.json())
app.post('/json/', function (req: any, res: any) {
    io.sockets.emit("publish", req.body);
    res.send("ok")
});
app.post('/write/', function (req: any, res: any) {
    const file: string = req.body.file;
    const remIndexes: number[] = req.body.remIndexes;
    fs.writeFile(file, remIndexes.map(x => `${x}`).join(" "), (err: ErrnoException | null) => {
        if (err) {
            res.send(JSON.stringify(err));
            return;
        }
        res.send("ok")
    });
});

app.get('/', function (req: any, res: any) {
    res.sendFile(__dirname + '/index.html');
});

http.listen(port, () => {
    console.info('Backend started on port: ' + port);
});
