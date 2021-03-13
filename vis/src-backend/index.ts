import * as express from "express";
import ErrnoException = NodeJS.ErrnoException;

const app = require('express')();
const http = require('http').Server(app);
const io = require('socket.io')(http);
const fs = require('fs');

const port = Number(process.env.PORT || 8888);
app.use(express.json())

interface JsonPublishRequest {
    body: any; // どんな json content でもOK
}

app.post('/json/publish', function (req: JsonPublishRequest, res: any) {
    io.sockets.emit("publish", req.body);
    res.send("ok")
});

interface WriteRequest {
    body: {
        file: string
        fileContent: string
    }
}


// セキュリティ的によろしくない(任意パスに任意内容を書き込める)のでインターネットに公開しないこと
app.post('/write/', function (req: WriteRequest, res: any) {
    const file: string = req.body.file;
    fs.writeFile(file, req.body.fileContent, (err: ErrnoException | null) => {
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
